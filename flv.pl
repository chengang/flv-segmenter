#!/usr/bin/perl

=pod

Dump Adobe flv file Info per frame.

Author: ChenGang
Corp: SINA
At 2013/4/7 Beijing

=cut

use strict;
use warnings;
use feature 'say';
use Term::ANSIColor::Print;

my %tag_type = (
    8  => 'audio',
    9  => 'video',
    18 => 'script',
);

my %audio_format = (
    0  => 'uncompressed',
    1  => 'ADPCM',
    2  => 'MP3',
    3  => 'Linear_PCM_little_endian',
    4  => 'Nellymoser_16kHz_mono',
    5  => 'Nellymoser_8kHz_mono',
    6  => 'Nellymoser',
    7  => 'G.711_A-law',
    8  => 'G.711_mu-law',
    10 => 'AAC',
    11 => 'Speex',
    14 => 'MP3_8kHz',
    15 => 'Device-specific_sound',
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
    2 => 'Sorenson_H.263',
    3 => 'Screen_video',
    4 => 'On2_VP6',
    5 => 'On2_VP6_alpha',
    6 => 'Screen_video_v2',
    7 => 'AVC',
);

my %video_type = (
    1 => 'keyframe',
    2 => 'interframe',
    3 => 'disposable_interframe',
    4 => 'generated_keyframe',
    5 => 'video_info/command_frame',
);

my %avc_packet_type = (
    0 => 'avc_seq_header',
    1 => 'avc_nalu',
    2 => 'avc_seq_end',
);

open FH, "<", shift;
binmode FH;
my $buf;
my $print = Term::ANSIColor::Print->new();

sysread( FH, $buf, 3 );
say "File Header: " . $buf;

sysread( FH, $buf, 1 );
say "Version: " . unpack( "C", $buf );

sysread( FH, $buf, 1 );    # jump Type Flags

sysread( FH, $buf, 4 );
say "Header Size: " . unpack( "N", $buf );

sysread( FH, $buf, 4 );    # jump PreviousTagSize0

my $tag_id = 0;

say "ID\tType\tSize\tTimestamp\tFormat\tExt";
while ( sysread( FH, $buf, 8 ) ) {
    $tag_id++;

    my ( $tag_type, $data_size, $ts, @datasize, @timestamp );
    (
        $tag_type,     $datasize[0],  $datasize[1],  $datasize[2],
        $timestamp[1], $timestamp[2], $timestamp[3], $timestamp[0]
    ) = unpack 'CCCCCCCC', $buf;

    $data_size = ( $datasize[0] * 256 + $datasize[1] ) * 256 + $datasize[2];
    $ts =
      ( ( $timestamp[0] * 256 + $timestamp[1] ) * 256 + $timestamp[2] ) * 256 +
      $timestamp[3];

    sysread( FH, $buf, 3 );            # jump SteamID, Always 0.
    sysread( FH, $buf, $data_size );

    $tag_type = $tag_type{$tag_type};
    my $info = $tag_id . "\t" . $tag_type . "\t" . $data_size . "\t" . $ts;

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
        $info .= "\t$audio_format\t$audio_type\t$audio_rate\t$audio_size";
        $print->yellow($info);
    }
    elsif ( $tag_type eq 'video' ) {
        my $flags       = unpack 'C', substr( $buf, 0, 1 );
        my $type        = ( $flags >> 4 ) & 0x0f;
        my $codec       = $flags & 0x0f;
        my $video_type  = $video_type{$type};
        my $video_codec = $video_codec{$codec};
        $info .= "\t$video_codec\t$video_type";
        if ( $video_codec eq 'AVC' ) {
            my @avc_time;
            my $avc_header = substr( $buf, 1, 4 );
            my $avc_packet_type;
            ( $avc_packet_type, $avc_time[0], $avc_time[1], $avc_time[2] ) =
              unpack 'CCCC', $avc_header;
            my $composition_time =
              ( $avc_time[0] * 256 + $avc_time[1] ) * 256 + $avc_time[2];
            $avc_packet_type = $avc_packet_type{$avc_packet_type};
            $info .= "\t$avc_packet_type\t$composition_time";
        }
        $print->red($info) unless $video_type eq 'keyframe';
        $print->red_on_white($info) if $video_type eq 'keyframe';
    }
    else {
        $print->green($info);
    }

    sysread( FH, $buf, 4 );    # jump PreviousTagSize
}

