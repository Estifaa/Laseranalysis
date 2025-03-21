#!/usr/bin/env perl

# The matricies must be multiplied in reverse order of spacial occurrence.
# The thickness of lenses must be accounted for when constructing compound objects.
# Note that the results are meaningless if the light rays miss either lens.
#
# coordinate system:
#
# X - along laser beam axis
# Y - vertical axis; positive downwards towards cathode
# Z - axis out of image display plane
#
# ToDo:
#	

# The aluminum strip positions for the final detector
# are, measured from the symmetry centre:
#       0 mm
# +-  360 mm
# +-  720 mm
# +- 1140 mm

use strict;
use warnings;

use GD::Simple;
use Math::BigFloat;
use Math::Matrix;
use Math::Random qw(random_normal);
use Math::Random qw(random_uniform);
use Math::Trig;
use Statistics::Histogram;

sub Vector($$$);		# r,a,v
sub FreeSpace($$);		# d,v
sub ThinLens($$);		# fl,v
sub ThickLens($$$$$$);		# n1,n2,R1,R2,t,v
sub FocalLength($$$$$);		# n2,R1,R2,t,v
sub MatrixMult(@);		# M1,M2,...,v
sub isnum($);			# arg
sub DrawLens($$$$$$);		# img,R1,R2,t,DL,d

my $YHIST = 0;			# flag to draw y profile histogram
my $AHIST = 1;			# flag to draw angle histogram
my $CXHIST = 1;			# flag to draw cathode histogram
my $Y7HIST = 0;			# flag to draw y6 profile histogram (endplate)
my $GD = 1;			# flag to draw ray image
my $PENCIL = 0;			# narrow test beam down the center 
my $CXDAT = 1;

my $inf;			# create an inf for ThickLens plano surfaces
if ($GD) {
  $inf = 9.e99;			# GD can't handle binf()
} else {
  $inf = Math::BigFloat->binf();	
}

my $VIEWER;
if ($^O eq "darwin") {
  #$VIEWER = "eog";		# use eog to view images
  $VIEWER = "xv";		# use xv to view images
} else {
  $VIEWER = "xdg-open";		# use xdg-open to view images
}

#my $nTrials = 1;		# pof test
#my $nTrials = 100;		# pof test
my $nTrials = 100000;		# pof test
#my $nTrials = 100000;		# number of trials to run
my $pTrials = 100;		# number of trials to plot
my $UseLinearAxes = 1;		# not a log plot...
my $UseIntegralBins = 0;	# using 1 causes get_histogram to hang sometimes 
my $nbins = 50;			# number of histogram bins

my @ydata;			# y data array for histograms
my @adata;			# a data array for histograms
my @cxdata;			# cathode x data array
my @cydata;			# cathode y data array
my @y7data;			# y7 data array for histograms (endplate)
my @xFiber;			# x coordinate at fiber
my @yFiber;			# y coordinate at fiber
my @xLens;			# x coordinate at lens centre
my @yLens;			# y coordinate at lens centre
my @xCol;			# x coordinate at front face of collimator
my @yCol;			# y coordinate at front face of collimator
my @xRod;			# x coordinate at quartz rod
my @yRod;			# y coordinate at quartz rod
my @xRod2;			# x coordinate at quartz rod exit
my @yRod2;			# y coordinate at quartz rod exit
my @xCath;			# x coordinate at cathode
my @yCath;			# y coordinate at cathode
my @alStrips;			# array of aluminum strip x coordinates 

my $drift = 73.0;		# TPC cathode-to-anode drift distance (mm)
my $height = 27.2;		# centre of quartz rod above cathode plane (mm)
my $lenTPC = 2354;		# TPC inner volume length
my $Fiber2TPC = 29.4;		# fiber end to inner TPC wall (mm)
my $alStripWid = 6;		# aluminum strip width

my $rodD = 5;			# quartz rod diameter (mm)

my $centerTPC = $lenTPC/2 + $Fiber2TPC;	# relative to end of fiber
push @alStrips, $centerTPC - 1140;
push @alStrips, $centerTPC - 720;
push @alStrips, $centerTPC - 360;
push @alStrips, $centerTPC;
push @alStrips, $centerTPC + 360;
push @alStrips, $centerTPC + 720;
push @alStrips, $centerTPC + 1140;

my ($rRand,$ryRand);		# random fiber light radius exit numbers
my ($aRand,$ayRand);		# random fiber light angle exit numbers
my ($phiRand);			# random fiber light radius and angle phi exit numbers

# optional argument list - use non-numeric placeholder to take default

my $def_frac = 0.22;	# default fraction of TPC to plot
my $def_FL = 12;	# default focal length
my $def_d1 = 10.28;	# default fiber to upstream lens face (mm)
my $def_d2 = 4;		# default lens back face to collimator distance (mm)
#my $def_d4 = 15;	# default lens back face to upstream rod face distance (mm)
my $def_d4 = 17.6;	# default lens back face to upstream rod face distance (mm) (from PB's drawings)
my $def_rodOff = 0.5;	# y offset of laser beam relative rod (mm); positive value results in downward shift of rod
my $def_coreD = 0.1;	# default fiber core diameter (mm)
my $def_NA = 0.22;	# default fiber numerical aperture (radians)
my $def_aSig = $def_NA;	# default sigma for fiber light angular distribution (radians)
my $def_holeD = 3;	# default collimator hole diameter (mm)
my $def_holeL = 5;	# default collimator hole length (mm)

my $flatDist = 0;       # 1 to use uniform light distribution over fiber surface, 0 for normal distribution

my $frac = $def_frac;		# optional arg  1 - fraction of TPC to plot
my $FL=$def_FL;			# optional arg  2 - focal length chooser (6, 9, 12, 18, or 24)
my $d1=$def_d1;			# optional arg  3 - length of free space 1
my $d2=$def_d2;			# optional arg  4 - length of free space 2
my $d4=$def_d4;			# optional arg  5 - length of free space 2
my $rodOff = $def_rodOff;	# optional arg  6 - quartz rod offset
my $coreD=$def_coreD;		# optional arg  7 - fiber core diameter
my $NA=$def_NA;			# optional arg  8 - fiber numerical aperture
my $aSig = $def_aSig;		# optional arg  9 - sigma for fiber light angular distribution
my $holeD=$def_holeD;		# optional arg 10 - collimator hole diameter
my $holeL=$def_holeL;		# optional arg 11 - collimator hole length

my $rSig = $coreD;	# sigma for fiber light radial distribution


if (@ARGV>0) {
  $frac = shift @ARGV;
  if (!isnum($frac)) {$frac = $def_frac;}
}
if (@ARGV>0) {
  $FL = shift @ARGV;
  if (!isnum($FL)) {$FL = $def_FL;}
}
if (@ARGV>0) {
  $d1 = shift @ARGV;
  if (!isnum($d1)) {undef $d1;}
}
if (@ARGV>0) {
  $d2 = shift @ARGV;
  if (!isnum($d2)) {$d2 = $def_d2;}
}
if (@ARGV>0) {
  $d4 = shift @ARGV;
  if (!isnum($d4)) {$d4 = $def_d4;}
}
if (@ARGV>0) {
  $rodOff = shift @ARGV;
  if (!isnum($rodOff)) {$rodOff = $def_rodOff;}
}
if (@ARGV>0) {
  $coreD = shift @ARGV;
  if (!isnum($coreD)) {$coreD = $def_coreD;}
}
if (@ARGV>0) {
  $NA = shift @ARGV;
  if (!isnum($NA)) {$NA = $def_NA;}
}
if (@ARGV>0) {
  $aSig = shift @ARGV;
  if (!isnum($aSig)) {$aSig = $def_aSig;}
}
if (@ARGV>0) {
  $holeD = shift @ARGV;
  if (!isnum($holeD)) {$holeD = $def_holeD;}
}
if (@ARGV>0) {
  $holeL = shift @ARGV;
  if (!isnum($holeL)) {$holeL = $def_holeL;}
}

# thick lens
my $n1 = 1.00;		# refractive index of air
my $n2 = 1.4997;	# refractive index of fused quartz at 266 nm
#my $n2 = 1.4574;	# refractive index of fused quartz at 620 nm (near end of red)
#my $n2 = 1.4542;	# refractive index of fused quartz at 750 nm (far end of red)
#my $n2 = 1.4585;	# refractive index of fused quartz at 587.6 nm (lens spec wavelength)
my ($R1,$R2,$t,$DL);
if ($FL==6) {
  $R1 =   4.76;		# radius of curvature surface 1 (mm)
  $R2 =  -4.76;		# radius of curvature surface 2 (mm)
  $t  =   4.10;		# centre thickness (mm)
  $DL =   6.00;		# lens diameter (mm)
} elsif ($FL==9) {
  $R1 =   7.80;		# radius of curvature surface 1 (mm)
  $R2 =  -7.80;		# radius of curvature surface 2 (mm)
  $t  =   2.70;		# centre thickness (mm)
  $DL =   6.00;		# lens diameter (mm)
} elsif ($FL==12) {
  $R1 =  10.65;		# radius of curvature surface 1 (mm)
  $R2 = -10.65;		# radius of curvature surface 2 (mm)
  $t  =   2.20;		# centre thickness (mm)
  $DL =   6.00;		# lens diameter (mm)
} elsif ($FL==18) {
  $R1 =   16.18;	# radius of curvature surface 1 (mm)
  $R2 =  -16.18;	# radius of curvature surface 2 (mm)
  $t  =    2.00;	# centre thickness (mm)
  $DL =    6.00;	# lens diameter (mm)
} elsif ($FL==24) {
  $R1 =   21.69;	# radius of curvature surface 1 (mm)
  $R2 =  -21.69;	# radius of curvature surface 2 (mm)
  $t  =    2.00;	# centre thickness (mm)
  $DL =    6.00;	# lens diameter (mm)
} else {
  printf "unknown lens...\n";
  exit;
}

#
### define the lens and also get some lens parameters
#
my $L1 = ThickLens($n1,$n2,$R1,$R2,$t,0);
my ($efl,$ffd,$bfd) = FocalLength($n2,$R1,$R2,$t,0);

#
### define the quartz rod
#
my $L2 = ThickLens($n1,$n2,$rodD/2,-$rodD/2,$rodD,0);

# loop over initial and final spacial conditions

for (my $i=0; $i<$nTrials; $i++) {

  #
  ### Initial random space and angle vectors:  Since the ray tracing is done
  ### only in two dimensions (in the x,y plane) the random space and angle vectors
  ### are produced in three dimensions, and then projection onto the x,y plane
  ### (equivalent to integrating along the z axis).  The three dimensional space 
  ### coordinate is randomized with a Gaussian distribution in r (in the y,z plane, 
  ### x being fixed) and also with a uniformly random azimuthal angle phi about the
  ### x axis.  The y component ryRand of that space point is then calculated and
  ### used as the initial space component for the ray tracing.  A random three
  ### dimensional angle is produced by choosing a polar angle theta relative to
  ### the x (laser beam) axis, randomized with a Gaussian distribution, and a
  ### uniformly random azimuthal angle about the x axis.  The resultant angle
  ### ayRand relative to the y axis in the x,y plane is then calculated and
  ### used as the initial angle component for the ray tracing.  With the initial
  ### ray tracing vector chosen in this manner, all coordinate results downstream
  ### are already projected onto the x,y plane as desired.
  #

  if($flatDist){
    for (;;) {
      $ryRand = random_uniform(1,-$coreD/2,$coreD/2);
      my $rzRand = random_uniform(1,-$coreD/2,$coreD/2);
      if (sqrt($ryRand*$ryRand + $rzRand*$rzRand) <= $coreD/2) {last;}
    }
  } else {
    for (;;) {
      $rRand = random_normal(1,0,$rSig);
      if (abs($rRand) <= $coreD/2) {last;}
    }
    $phiRand = 2*pi * rand();
    $ryRand = $rRand * sin($phiRand);
  }

  for (;;) {
    $aRand = random_normal(1,0,$aSig);
    #$aRand = random_uniform(1,0,0.05);
    if (abs($aRand) <= $NA) {last;}
  }
  $phiRand = 2*pi * rand();
  $ayRand = atan(sin($phiRand)*tan($aRand));

  if ($PENCIL) {
    $ryRand = 0;
    $ayRand = 0;
  }

  my $V1 = Vector($ryRand,$ayRand,0);

  #
  ### upstream free space
  #
  if (!defined($d1)) {$d1 = $ffd;}	# length of free space 1 (mm) (ffd)
  my $S1 = FreeSpace($d1,0);

  #
  ### downstream free space to front end of collimator
  #
  my $S2 = FreeSpace($d2,0);
  # do the matrix multiplication
  my $V2 = MatrixMult($S2,$L1,$S1,$V1,0);
  # get the collimater front end results
  my $y2 = $V2->[0]->[0];
  my $a2 = $V2->[1]->[0];

  #
  ### downstream free space to back end of collimator
  #
  my $d3 = $d2 + $holeL;
  my $S3 = FreeSpace($d3,0);
  # do the matrix multiplication
  my $V3 = MatrixMult($S3,$L1,$S1,$V1,0);
  # get the collimater back end results
  my $y3 = $V3->[0]->[0];
  my $a3 = $V3->[1]->[0];

  #
  ### downstream free space to rod
  #
  my $S4 = FreeSpace($d4,0);
  # do the matrix multiplication
  my $V4 = MatrixMult($S4,$L1,$S1,$V1,0);

  #
  ### get the rod results
  #
  my $y4 = $V4->[0]->[0];
  my $a4 = $V4->[1]->[0];
  if (abs($y2)<$holeD/2 && abs($y3)<$holeD/2) {
    push @ydata, $y4;		# fill the output y data array relaive to laser axis
    push @adata, rad2deg($a4);	# fill the output a data array
  }

  push @xFiber, 0;		# x coordinate at fiber
  push @yFiber, $ryRand;	# y coordinate at fiber

  my $x1 = $d1 + $t/2;	
  my $y1 = $ryRand + $x1*tan($ayRand);
  push @xLens, $x1;		# x coordinate at lens centre
  push @yLens, $y1;		# y coordinate at lens centre

  my $x2 = $d1 + $t + $d2;	# collimator front face
  push @xCol, $x2;		# x coordinate at front face of collimator
  push @yCol, $y2;		# y coordinate at front face of collimator

  my $x4 = $d1 + $t + $d4;
  if (abs($y2)<$holeD/2 && abs($y3)<$holeD/2) {
    push @xRod, $x4;		# x coordinate at quartz rod
    push @yRod, $y4;		# y coordinate at quartz rod
  } else {
    push @xRod, -999;		# flag the rays that don't 
    push @yRod, -999;		# get through collimator
  }

  #
  ### offset the beam before entering the rod
  #
  my $V5 = Vector($y4-$rodOff,$a4,0);

  #
  ### get the cathode results
  #
  my $V6 = MatrixMult($L2,$V5,0);
  my $x6 = $d1 + $t + $d4 + $rodD;		# x coordinate at quartz rod exit
  my $y6 = $V6->[0]->[0];			# y6 is relative to rod centre; positive toward cathode
  my $a6 = $V6->[1]->[0];			# a6 > 0 => rays directed toward cathode

  if (abs($y2)<$holeD/2 && abs($y3)<$holeD/2) {
    if ($a6>0) {
      my $cx = ($height-$y6)*cot($a6);
      if ($cx>0 && $cx<$lenTPC) {
        push @cxdata, $cx;				# cathode histogram
        push @cydata, -1*$height;
      }
    }
  }

  if (abs($y2)<$holeD/2 && abs($y3)<$holeD/2) {
    push @xRod2, $x6;					# x coordinate at quartz rod exit
    push @yRod2, $y6;					# y coordinate at quartz rod exit

    if ($a6>=0) {
      my $cx = ($height-$y6)*cot($a6);			# x coordinate at cathode
      my $cy = $height;					# y coordinate of cathode (positive)
      push @xCath, $cx;
      push @yCath, $cy;
    } else {
      my $cx = (($drift-$height)+$y6)*cot(-$a6);	# x coordinate at anode
      my $cy = $height-$drift;				# y coordinate of anode (negative)
      push @xCath, $cx;
      push @yCath, $cy;
    }

  } else {
    push @xRod2, -999;		# flag the rays that don't 
    push @yRod2, -999;		# get through collimator
    push @xCath, -999;
    push @yCath, -999;
  }

  #
  ### downstream free space from rod as far as end of TPC
  #
  my $S5 = FreeSpace($lenTPC,0);

  # do the matrix multiplication
  my $V7 = MatrixMult($S5,$L2,$V5,0);

  #
  ### get the endplate results
  #
  my $y7 = $V7->[0]->[0];
  if (abs($y2)<$holeD/2 && abs($y3)<$holeD/2) {
    if ($y7 > -1*$height && $y7 < ($drift-$height)) {
      push @y7data, $y7+$height;				# endplate histogram
    }
  }
}

# shift the y4 distribution at the rod to be relative to rod centre
@ydata = map { $_ + $rodOff } @ydata;

# rescale and shift the plot arrays

my $xSize = 2*640;	# x size of ray trace plot
my $ySize = 2*480;	# y size of ray trace plot
my $xBorder = 50;	# x border size
my $yBorder = 100;	# y border size

# calculate the pixels/mm scale factor; keep aspect ratio = 1
my $scale = ($xSize-2*$xBorder)/($frac*$lenTPC);

my $xOff = $xBorder;
my $yOff = $ySize/2;

# convert to image coordinates
@xFiber = map { $_ * $scale + $xOff } @xFiber;
@yFiber = map { $_ * $scale + $yOff } @yFiber;
@xLens = map { $_ * $scale + $xOff } @xLens;
@yLens = map { $_ * $scale + $yOff } @yLens;
@xCol = map { $_ * $scale + $xOff } @xCol;
@yCol = map { $_ * $scale + $yOff } @yCol;
@xRod = map { $_ * $scale + $xOff } @xRod;
@yRod = map { $_ * $scale + $yOff } @yRod;
@xRod2 = map { $_ * $scale + $xOff } @xRod2;
@yRod2 = map { $_ * $scale + $yOff } @yRod2;
@xCath = map { $_ * $scale + $xOff } @xCath;
@yCath = map { $_ * $scale + $yOff } @yCath;
@alStrips = map { $_ * $scale + $xOff } @alStrips;

my $colX1 = $xOff + $scale*($d1+$t+$d2);
my $colY1 = $yOff - $scale*$holeD/2;
my $colX2 = $xOff + $scale*($d1+$t+$d2+$holeL);
my $colY2 = $yOff + $scale*$holeD/2;
my $colY1up = $colY1 - $scale*($DL-$holeD)/2;
my $colY2dn = $colY2 + $scale*($DL-$holeD)/2;

my $rodX = $xOff + $scale*($d1+$t+$d4+$rodD/2);
my $rodY = $yOff + $scale*$rodOff;
my $rodDx = $scale*$rodD;
my $rodDy = $scale*$rodD;

my $tpcCathodeY = $yOff + $scale*$height;
my $tpcAnodeY = $yOff - $scale*($drift-$height);

my $tpcX1 = $xOff + $scale*$Fiber2TPC;
my $tpcY1 = $tpcCathodeY;
my $tpcX2 = $xOff + $scale*$Fiber2TPC + $scale*$lenTPC;
my $tpcY2 = $tpcCathodeY;
my $tpcX3 = $xOff + $scale*$Fiber2TPC + $scale*$lenTPC;
my $tpcY3 = $tpcAnodeY;
my $tpcX4 = $xOff + $scale*$Fiber2TPC;
my $tpcY4 = $tpcAnodeY;

my $StripWidScaled = $scale * $alStripWid;

printf "\n";
printf " 1) fraction of TPC to plot			 = %6.2f\n", $frac;
printf " 2) advertised focal length                      = %6.2f mm\n", $FL;
printf " 3) fiber to upstream lens face                  = %6.2f mm\n", $d1;
printf " 4) downstream lens face to collimator           = %6.2f mm\n", $d2;
printf " 5) downstream lens face to rod centre           = %6.2f mm\n", $d4;
printf " 6) rod offset (positive is downward shift)	 = %6.2f mm\n", $rodOff;
printf " 7) fibre core diameter                          = %6.2f mm\n", $coreD;
printf " 8) fiber numerical aperture                     = %6.2f radians\n", $NA;
printf " 9) sigma for fiber light angular distribution   = %6.2f radians\n", $aSig;
printf "10) collimator hole diameter                     = %6.2f mm\n", $holeD;
printf "11) collimator hole length                       = %6.2f mm\n", $holeL;
printf " *) lens thickness at centre                     = %6.2f mm\n", $t;

if ($YHIST && @ydata) {
  printf "\ny profile distribution at quartz rod (mm)\n";
  print get_histogram(\@ydata, $nbins, $UseLinearAxes, $UseIntegralBins);
}
if ($AHIST && @adata) {
  printf "\nangle distribution at quartz rod (degrees)\n";
  print get_histogram(\@adata, $nbins, $UseLinearAxes, $UseIntegralBins);
}
if ($CXHIST && @cxdata) {
  printf "\nx profile distribution on cathode (mm)\n";
  print get_histogram(\@cxdata, $nbins, $UseLinearAxes, $UseIntegralBins);
}
if ($Y7HIST && @y7data) {
  printf "\ny profile distribution at endplate (mm)\n";
  print get_histogram(\@y7data, $nbins, $UseLinearAxes, $UseIntegralBins);
}

if ($CXDAT) {
  print	"Trying to write to file\n";	
  open my $out, '>', 'cx.dat' or die "Cannot open\n";
  foreach (@cxdata){
      print $out "$_\n";
  }
}

if ($GD) {
  my $img = GD::Simple->new($xSize,$ySize);
  my ($x,$y);

  # draw the fiber
  $img->fgcolor('green');
  $img->penSize(3,3);
  $x = 0;
  $y = -$scale*$coreD/2 + $yOff;
  $img->moveTo($x,$y);
  $x = $xBorder;
  $img->lineTo($x,$y);
  $x = 0;
  $y =  $scale*$coreD/2 + $yOff;
  $img->moveTo($x,$y);
  $x = $xBorder;
  $img->lineTo($x,$y);

  # draw the collimating lens
  DrawLens($img,$R1,$R2,$t,$DL,$d1);

  # draw the light rays from the fiber to the quartz rod
  $img->fgcolor('blue');
  $img->penSize(1,1);
  for (my $i=0; $i<$pTrials; $i++) {
    if (!@xFiber || !@yFiber) {last;}

    $x = shift @xFiber;
    $y = shift @yFiber;
    #printf "%.2f %.2f ", $x,$y;
    $img->moveTo($x,$y);

    $x = shift @xLens;
    $y = shift @yLens;
    #printf "%.2f %.2f ", $x,$y;
    $img->lineTo($x,$y);

    $x = shift @xCol;
    $y = shift @yCol;
    #printf "%.2f %.2f ", $x,$y;
    $img->lineTo($x,$y);

    $x = shift @xRod;
    $y = shift @yRod;
    #printf "%.2f %.2f\n", $x,$y;
    if ($x>0) {
      $img->lineTo($x,$y);
    } else {
      next;
    }

    $x = shift @xRod2;
    $y = shift @yRod2;
    #printf "%.2f %.2f\n", $x,$y;
    if ($x>0) {
      $img->lineTo($x,$y);
    } else {
      next;
    }

    $x = shift @xCath;
    $y = shift @yCath;
    #printf "%.2f %.2f\n", $x,$y;
    if ($x>0) {
      $img->lineTo($x,$y);
    } else {
      next;
    }
  }

  # draw the collimator
  $img->fgcolor('green');
  $img->penSize(3,3);
  $img->moveTo($colX1,$colY1);
  $img->lineTo($colX2,$colY1);
  $img->moveTo($colX1,$colY1);
  $img->lineTo($colX1,$colY1up);

  $img->moveTo($colX1,$colY2);
  $img->lineTo($colX2,$colY2);
  $img->moveTo($colX1,$colY2);
  $img->lineTo($colX1,$colY2dn);

  # draw the quartz rod
  $img->fgcolor('red');
  $img->penSize(3,3);
  $img->moveTo($rodX,$rodY);
  $img->ellipse($rodDx, $rodDy);

  # draw the TPC
  $img->fgcolor('black');
  $img->penSize(3,3);
  $img->moveTo($tpcX1,$tpcY1);
  $img->lineTo($tpcX2,$tpcY2);
  $img->lineTo($tpcX3,$tpcY3);
  $img->lineTo($tpcX4,$tpcY4);
  $img->lineTo($tpcX1,$tpcY1);

  # draw the aluminum strips
  $img->fgcolor('red');
  $img->penSize(5,5);
  foreach my $strip (@alStrips) {
    $img->moveTo($strip-$StripWidScaled/2, $tpcCathodeY);
    $img->lineTo($strip+$StripWidScaled/2, $tpcCathodeY);
  }

  open my $out, '>', 'LaserTraces.png' or die;
  binmode $out;
  print $out $img->png;
  my $cmd = $VIEWER . " LaserTraces.png &";
  system ($cmd);
}

#
########################################################################
#
# Calculate and return the matrix representation for a spacial vector
#
sub Vector($$$) {
  my $r    = shift;			# offset from optical axis
  my $a    = shift;			# angle (dr/dz) from optical axis
  my $verb = shift;			# verbose

  my $V = new Math::Matrix ([$r],[$a]);

  if ($verb) {
    printf "\nVector:\n";
    $V->print("\n");
  }

  return $V;
}

#
########################################################################
#
# Calculate and return the matrix representation for a thin lens
#
sub ThinLens($$) {
  my $f    = shift;			# focal length
  my $verb = shift;			# verbose

  my $L = new Math::Matrix ([1,0],[-1/$f,1]);

  if ($verb) {
    printf "\nThinLens:\n";
    $L->print("\n");
  }

  return $L;
}

#
########################################################################
#
# Calculate and return the matrix representation for a thick lens;
# pass r = $inf for plano surfaces
#
sub ThickLens($$$$$$) {
  my $n1   = shift;			# refractive index outside of lens
  my $n2   = shift;			# refractive index inside lens
  my $r1   = shift;			# radius of curvature first surface
  my $r2   = shift;			# radius of curvature second surface
  my $t    = shift;			# thickness at centre
  my $verb = shift;			# verbose

  my $L1 = new Math::Matrix ([1,0],[($n1-$n2)/($r1*$n2),$n1/$n2]);
  my $LC = new Math::Matrix ([1,$t],[0,1]);
  my $L2 = new Math::Matrix ([1,0],[($n2-$n1)/($r2*$n1),$n2/$n1]);
  my $L = MatrixMult($L2,$LC,$L1,0);

  if ($verb) {
    printf "\nThickLens:\n";
    $L->print("\n");
  }

  return $L;
}

#
########################################################################
#
# Calculate and return some focal length parameters;
# pass r = $inf for plano surfaces
#
sub FocalLength($$$$$) {
  my $n    = shift;			# refractive index inside lens
  my $r1   = shift;			# radius of curvature first surface
  my $r2   = shift;			# radius of curvature second surface
  my $t    = shift;			# thickness at centre
  my $verb = shift;			# verbose

  my $efl = 1 / ( ($n-1) * ( (1/$r1) - (1/$r2) + ($n-1)*$t/($n*$r1*$r2) ) );
  my $ffd = $efl * ( 1 + ($n-1)*$t/($n*$r2) );
  my $bfd = $efl * ( 1 - ($n-1)*$t/($n*$r1) );

  if ($verb) {
    print "\nFocalLength:\n\n";
    printf "efl = %.2f mm\n", $efl;
    printf "ffd = %.2f mm\n", $ffd;
    printf "bfd = %.2f mm\n", $bfd;
  }

  return ($efl,$ffd,$bfd);
}

#
########################################################################
#
# Calculate and return the matrix representation for free space
#
sub FreeSpace($$) {
  my $d = shift;			# length of free space
  my $verb = shift;			# verbose

  my $S = new Math::Matrix ([1,$d],[0,1]);

  if ($verb) {
    printf "\nFreeSpace:\n";
    $S->print("\n");
  }

  return $S;
}

#
########################################################################
#
# Matrix multiplication, two or more arguments
#
sub MatrixMult(@) {
  my $v = pop @_;		# pull the last element off the array
  my $M = shift @_;		# pull first element off array
  #$M->print("\n");
  foreach my $m (@_) {
    $M = $M->multiply($m);	# multiplies (M) X (m)
    if ($v) {
      printf "\nMatrixMult:\n";
      $M->print("\n");
    }
  }
  
  return $M;
}

#
########################################################################
#
# Check if argument is numeric
#
sub isnum($) {
  no warnings;
  use warnings FATAL => 'numeric';
  return defined eval { $_[ 0] == 0 };
}

#
########################################################################
#
# Draw a lens
#
sub DrawLens($$$$$$) {

  my $img = shift;		# image handle
  my $Ru  = shift;		# radius of upstream lens surface (mm)
  my $Rd  = shift;		# radius of downstream lens surface (mm)
  my $t   = shift;		# centre thickness of lens (mm)
  my $DL  = shift;		# diameter of lens (mm)
  my $d   = shift;		# disance from xOff to front face of lens (mm)

  $img->fgcolor('red');
  $img->penSize(3,3);

  my ($x,$y);
  my ($wu,$wd);			# upstream and downstream arc widths
  my ($xuhi,$yuhi);		# top corner of lens upstream face in image coordinates
  my ($xdhi,$ydhi);		# top corner of lens downstream face in image coordinates
  my ($xuc,$yuc);		# centre of upstream arc in image coordinates
  my ($xdc,$ydc);		# centre of downstream arc in image coordinates
  my ($arcu,$arcd);

  # calculate the lens circle widths in pixels
  if ($Ru<$inf) {
    $wu = 2*abs($scale*$Ru);		
  } else {
    $wu = -99;
  }
  if ($Rd<$inf) {
    $wd = 2*abs($scale*$Rd);
  } else {
    $wd = -99;
  }
  #printf "wu = %.2f, wd = %.2f\n", $wu, $wd;

  # calculate the image coordinates of the upstream top corner
  if ($wu>0) {
    $xuhi = $xOff + $scale*$d + ($wu/2) - sqrt( ($wu/2)**2 - ($scale*$DL/2)**2 );
  } else {
    $xuhi = $xOff + $scale*$d;
  }
  $yuhi = $yOff - $scale*$DL/2;
  #printf "xuhi = %.2f, yuhi = %.2f\n", $xuhi, $yuhi;

  # calculate the image coordinates of the downstream top corner
  if ($wd>0) {
    $xdhi = $xOff + $scale*($d+$t) - ($wd/2) + sqrt( ($wd/2)**2 - ($scale*$DL/2)**2 );
  } else {
    $xdhi = $xOff + $scale*($d+$t);
  }
  $ydhi = $yOff - $scale*$DL/2;
  #printf "xdhi = %.2f, ydhi = %.2f\n", $xdhi, $ydhi;

  # calculate the upstream arc half angle
  if ($wu>0) {
    $xuc = $xOff + $scale*($d+$Ru);
    $yuc = $yOff;
    $arcu = rad2deg(atan(abs($yuhi-$yuc)/abs($xuhi-$xuc)));
    #printf "xuc = %.2f, yuc = %.2f, arcu = %.2f\n", $xuc, $yuc, $arcu;
  }

  # calculate the downstream arc half angle
  if ($wd>0) {
    $xdc = $xOff + $scale*($d+$t+$Rd);
    $ydc = $yOff;
    $arcd = rad2deg(atan(abs($ydhi-$ydc)/abs($xdhi-$xdc)));
    #printf "xdc = %.2f, ydc = %.2f, arcd = %.2f\n", $xdc, $ydc, $arcd;
  }

  # draw the upstream face
  if ($wu>0) {
    $img->moveTo($xuc,$yuc);
    $img->arc($wu,$wu,180-$arcu,180+$arcu,gdEdged);
  } else {
    $x = $xuhi;
    $y = $yOff + $scale*$DL/2;
    $img->moveTo($x,$y);
    $y = $yOff - $scale*$DL/2;
    $img->lineTo($x,$y);
  }

  # draw the downstream face
  if ($wd>0) {
    $img->moveTo($xdc,$ydc);
    $img->arc($wd,$wd,-$arcd,$arcd,gdEdged);
  } else {
    $x = $xdhi;
    $y = $yOff + $scale*$DL/2;
    $img->moveTo($x,$y);
    $y = $yOff - $scale*$DL/2;
    $img->lineTo($x,$y);
  }

  # fill in the blanks...
  $img->moveTo($xuhi,$yuhi);
  $x = $xdhi;
  $img->lineTo($x,$ydhi);

  $y = $yuhi + $scale*$DL;
  $img->moveTo($xuhi,$y);
  $x = $xdhi;
  $img->lineTo($x,$y);
}

#
########################################################################
#




