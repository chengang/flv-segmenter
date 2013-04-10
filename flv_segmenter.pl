#!/usr/bin/perl

=pod

Split Adobe flv file into segments at keyframes.

Author: ChenGang
Corp: SINA
At 2013/4/7 Beijing

=cut

use strict;

my %tag_type = (
    8  => 'audio',
    9  => 'video',
    18 => 'script',
);

my %audio_format = (
    0  => 'uncompressed',
    1  => 'ADPCM',
    2  => 'MP3',
    3  => 'Linear PCM, little endian',
    4  => 'Nellymoser 16kHz mono',
    5  => 'Nellymoser 8kHz mono',
    6  => 'Nellymoser',
    7  => 'G.711 A-law',
    8  => 'G.711 mu-law',
    10 => 'AAC',
    11 => 'Speex',
    14 => 'MP3 8kHz',
    15 => 'Device-specific sound',
);

my %audio_rate = (
    0 => '5518Hz',
    1 => '11025Hz',
    2 => '22050Hz',
    3 => '44100Hz',
);

my %audio_size = (
    0 => '8bit',
    1 => '16bit',
);

my %audio_type = (
    0 => 'mono',
    1 => 'stereo',
);

my %video_codec = (
    1 => 'JPEG',
    2 => 'Sorenson H.263',
    3 => 'Screen video',
    4 => 'On2 VP6',
    5 => 'On2 VP6 + alpha',
    6 => 'Screen video v2',
    7 => 'AVC',
);

my %video_type = (
    1 => 'keyframe',
    2 => 'interframe',
    3 => 'disposable interframe',
    4 => 'generated keyframe',
    5 => 'video info/command frame',
);

my $input_flv = shift;
open FH, "<", $input_flv;
binmode FH;
my $buf;
my $out_buf;
my $flv_header;
my $flv_script;
my $avc_0_frame;
my $aac_0_frame;
my $if_video_eq_avc;
my $if_audio_eq_aac;
my $i      = 1;
my $tag_id = 0;

sysread( FH, $buf, 13 );
$flv_header .= $buf;

while ( sysread( FH, $buf, 8 ) ) {
    $tag_id++;

    my $tag_data;
    my $video_type = "";

    $tag_data .= $buf;
    my ( $tag_type, $data_size, $ts, @datasize, @timestamp );
    (
        $tag_type,     $datasize[0],  $datasize[1],  $datasize[2],
        $timestamp[1], $timestamp[2], $timestamp[3], $timestamp[0]
    ) = unpack 'CCCCCCCC', $buf;

    $data_size = ( $datasize[0] * 256 + $datasize[1] ) * 256 + $datasize[2];
    $ts =
      ( ( $timestamp[0] * 256 + $timestamp[1] ) * 256 + $timestamp[2] ) * 256 +
      $timestamp[3];

    sysread( FH, $buf, 3 );    # jump SteamID, Always 0.
    $tag_data .= $buf;
    sysread( FH, $buf, $data_size );
    $tag_data .= $buf;

    $tag_type = $tag_type{$tag_type};

    if ( $tag_type eq 'audio' ) {
        my $flags = unpack 'C', substr( $buf, 0, 1 );
        my $format = ( ( $flags >> 4 ) & 0x0f );
        my $rate   = ( ( $flags >> 2 ) & 0x03 );
        my $size   = ( ( $flags >> 1 ) & 0x01 );
        my $type   = $flags & 0x01;
        my $audio_format = $audio_format{$format};
        my $audio_rate   = $audio_rate{$rate};       # Always 44100 when AAC
        my $audio_size   = $audio_size{$size};
        my $audio_type   = $audio_type{$type};

        if ( $audio_format eq 'AAC' ) {
            $if_audio_eq_aac = 1;
        }
    }
    elsif ( $tag_type eq 'video' ) {
        my $flags = unpack 'C', substr( $buf, 0, 1 );
        my $type  = ( $flags >> 4 ) & 0x0f;
        my $codec = $flags & 0x0f;
        $video_type = $video_type{$type};
        my $video_codec = $video_codec{$codec};
        if ( $video_codec eq 'AVC' ) {
            $if_video_eq_avc = 1;
        }
    }

    sysread( FH, $buf, 4 );    # jump PreviousTagSize
    $tag_data .= $buf;

    $avc_0_frame = $tag_data if ( !$avc_0_frame and $tag_type eq 'video' );
    $aac_0_frame = $tag_data if ( !$aac_0_frame and $tag_type eq 'audio' );
    if ( !$if_video_eq_avc or $tag_id > 3 ) {
        if ( $video_type eq 'keyframe' ) {
            my $out_file = $input_flv . "_" . $i++ . ".flv";
            open OUT, ">", $out_file;
            binmode OUT;
            print OUT $flv_header;
            print OUT $avc_0_frame if $if_video_eq_avc;
            print OUT $aac_0_frame if $if_audio_eq_aac;
            print "generating $out_file\n";
        }
        print OUT $tag_data;
    }
}

