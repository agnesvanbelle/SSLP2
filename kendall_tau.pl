#!/usr/bin/perl -w
use strict;

my $DEBUG_PERM = 0;

sub kendall_tau{
    my ($first_perm, $second_perm) = @_;
    my @first_perm = @$first_perm;
    my @second_perm = @$second_perm;
    my @first_inverted = invert(@first_perm);
    my @second_inverted = invert(@second_perm);

    my $penalty = 0;

    my $n = $#first_perm;
    for my $i (0..$n-1){
        for my $j ($i+1..$n){
            my $first_is_monotone;
            if($first_inverted[$i]>$first_inverted[$j] and
                    $second_inverted[$i]<$second_inverted[$j]){
                $penalty++;
                print STDERR "error in $i,$j\n" if $DEBUG_PERM;
            }elsif($first_inverted[$i]<$first_inverted[$j] and
                $second_inverted[$i]>$second_inverted[$j]){
                $penalty++;
                print STDERR "error in $i,$j\n" if $DEBUG_PERM;
            }
        }
    }
    my $Z = $n*($n+1)/2;
    my $tau = $penalty / $Z;
    return $tau;
}

sub invert{
    my @input = @_;
    my @inverted;
    for my $i (0..$#input){
        $inverted[$input[$i]]=$i;
    }
    return @inverted;
}


sub MAIN{
    my ($first_fn, $second_fn) = @_;

    open my $first_fh, $first_fn or die $!;
    open my $second_fh, $second_fn or die $!;

    my $total_tau = 0;
    my $count = 0;
    while(<$first_fh>){
        my $first_str = $_;
        chomp $first_str;
        my $second_str = <$second_fh>;
        chomp $second_str;

        my @first_perm = map {$_+0} split / +/, $first_str;
        my @second_perm = map {$_+0} split / +/, $second_str;

        my $tau = kendall_tau(\@first_perm, \@second_perm);
        $total_tau+=$tau;
        $count++;
    }

    print "Average (1-)Kendall's tau is ",(1-$total_tau/$count),"\n";
}

MAIN(@ARGV);


