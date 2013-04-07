use feature 'say';
use Term::ANSIColor::Print;

my %tag_type = 
(
	8  => 'audio',
	9  => 'video',
	18 => 'ScriptData',
);

open FH, "<", shift;
binmode FH;
my $buf;
my $print = Term::ANSIColor::Print->new();

sysread(FH, $buf, 3);
say "File Header: " . $buf;

sysread(FH, $buf, 1);
say "Version: " . unpack("C",$buf);

sysread(FH, $buf, 1); # jump Type Flags

sysread(FH, $buf, 4);
say "Header Size: " . unpack("N",$buf);

sysread(FH, $buf, 4); # jump PreviousTagSize0

print "Tag Type\tData Size\tTimestamp\n";
while( sysread(FH, $buf, 8) )
{
	my ($tag_type, $data_size, $ts, @datasize, @timestamp);
	(
	 $tag_type,         $datasize[0],  $datasize[1],  $datasize[2],
	 $timestamp[1], $timestamp[2], $timestamp[3], $timestamp[0]
	) = unpack 'CCCCCCCC', $buf;

	my $data_size = ($datasize[0] * 256 + $datasize[1]) * 256 + $datasize[2];
	my $ts = (($timestamp[0] * 256 + $timestamp[1]) * 256 + $timestamp[2]) * 256 + $timestamp[3];

	sysread(FH, $buf, 3); # jump SteamID, Always 0.
	sysread(FH, $buf, $data_size);

	$tag_type = $tag_type{$tag_type};
	my $info = $tag_type . "\t" . $data_size . "\t" . $ts;

	if ($tag_type eq 'audio')
	{
		#my $flag = 1;
		$print->yellow($info);
	}
	elsif ($tag_type eq 'video')
	{
		$print->red($info);
	}
	else
	{
		$print->green($info);
	}

	sysread(FH, $buf, 4); # jump PreviousTagSize
}


