#!/usr/bin/perl

#DFZ: parse the time stamps of each compute node on BGP and aggregate the throughput

$BS = 256;
$REP = 65;

%res = ();

$file = $ARGV[0];
open $fh, $file; 

while ($line = <$fh>)
{
	@words = split(/ +/, $line);
	for ($chunk = 2; $chunk <= $REP; $chunk++)
	{
		$tp = $BS / ($words[$chunk] - $words[$chunk - 1]);	
		for ($ts = (1 + $words[$chunk - 1]); $ts <= $words[$chunk]; $ts++)
		{
			$res{$ts} += $tp;
		}
	}
#	last if $. == 10;
}

foreach $key (sort keys %res)
{
	print "$key: $res{$key} \n";
}
