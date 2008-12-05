$/ = undef;
$_ = <>;

s!/\*.*?\*/!!sg;
s!\\\n! !sg;

@lines = grep {length} split "\n";

my $needline;
for (@lines) {
	$_ .= ' ';
	s/\s+/ /g;
	s/^\s*#\s*/#/;
	my $xline = /^\s?#/ && 1;
	if ($needline || $xline) {
		$_ = "\n" . $_;
	}
	$needline = $xline;
}

$_ = join "", @lines;

s/[ \t]+/ /sg;

print "$_\n";
