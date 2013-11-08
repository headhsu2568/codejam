#!/usr/bin/perl
  use strict;
  use warnings;

  srand(5);
  my $range = 90000000;
  #my $range = 10;  
  my $random_number0 = 0;
  my $random_number1 = 0;

#for (my $i = 0; $i < 10; $i++) {
for (my $i = 0; $i < 90000000; $i++) {
  $random_number0 = int(rand($range));
  $random_number1 = int(rand($range));
  print $random_number0 . "        " . $random_number1 . "\n";
  }
