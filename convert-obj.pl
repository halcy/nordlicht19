#!/usr/bin/perl

use warnings;
use strict;

my $scale = 0.4;
my $offset = 0;
my $suffix = "";

my @vertices;
my @faces;

my $material = 0;
my $lastmaterial = 0;
my %materials = ();

while(<>) {
        if( $_ =~ /usemtl ([^ ]+)/ ) {
            if(defined $materials{$1}) {
                $material = $materials{$1};
            }
            else {
                $material = $lastmaterial;
                $materials{$1} = $material;
                $lastmaterial++;
            }
        }
        
    if( $_ =~ /v ([^ ]+) ([^ ]+) ([^ ]+)/ ) {
        push @vertices, [$1, $2, $3];
    }

    # Assume normals are per face, no whatever calculate normals later
    if( $_ =~ /f ([0-9]+)\/[0-9]*\/([0-9]+) ([0-9]+)\/[0-9]*\/([0-9]+) ([0-9]+)\/[0-9]*\/([0-9]+)/ ) {
        push @faces, [$1, $3, $5, $2];
    }

    if( $_ =~ /f ([0-9]+) ([0-9]+) ([0-9]+)/ ) {
        push @faces, [$1, $2, $3];
    }
    
    if( $_ =~ /f ([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+)/ ) {
        push @faces, [$1, $2, $3, $4];
    }
    
# 	if( $_ =~ /vn ([^ ]+) ([^ ]+) ([^ ]+)/ ) {
#                 my $normalid = "$1__$2__$3__";
# 		push @normals, [$1, $2, $3];
# 	}
}

my %normals = ();
my @normalsarr = ();
my $normalidx = 1;
for(my $faceidx = 0; $faceidx < scalar @faces; $faceidx++) {
    my @face = @{$faces[$faceidx]};
    my @a = @{$vertices[$face[0] - 1 + $offset]};
    my @b = @{$vertices[$face[1] - 1 + $offset]};
    my @c = @{$vertices[$face[2] - 1 + $offset]};
    my @ab = ($b[0] - $a[0], $b[1] - $a[1], $b[2] - $a[2]);
    my $ablen = sqrt($ab[0] * $ab[0] + $ab[1] * $ab[1] + $ab[2] * $ab[2]);
    @ab = ($ab[0] / $ablen, $ab[1] / $ablen, $ab[2] / $ablen);
    my @ac = ($c[0] - $a[0], $c[1] - $a[1], $c[2] - $a[2]);
    my $aclen = sqrt($ac[0] * $ac[0] + $ac[1] * $ac[1] + $ac[2] * $ac[2]);
    @ac = ($ac[0] / $aclen, $ac[1] / $aclen, $ac[2] / $aclen);
    my @p = (
        $ab[1] * $ac[2] - $ab[2] * $ac[1],
        $ab[2] * $ac[0] - $ab[0] * $ac[2],
        $ab[0] * $ac[1] - $ab[1] * $ac[0]
    );
    my $plen = sqrt($p[0] * $p[0] + $p[1] * $p[1] + $p[2] * $p[2]);
    if($plen == 0) {
        splice(@faces, $faceidx, 1);
        $faceidx--;
        next;
    }
    @p = ($p[0] / $plen, $p[1] / $plen, $p[2] / $plen);
    my $normalid = $p[0] . "---" . $p[1] . "---" . $p[2];
    my $normalex = $normals{$normalid};
    my $normalexidx = 0;
    if(!defined $normalex) {
        $normalexidx = $normalidx;
        $normals{$normalid} = $normalidx;
        push @normalsarr, \@p;
        $normalidx++;
    }
    else {
        $normalexidx = $normalex;
    }
    
    $faces[$faceidx]->[3] = $normalexidx;
}

print "#define numVertices$suffix " . scalar @vertices . "\n";
print "#define numNormals$suffix " . scalar @normalsarr . "\n";
print "#define numFaces$suffix " . scalar @faces . "\n\n";

print "#include \"Rasterize.h\"\n\n";

print "const init_vertex_t vertices" . $suffix . "[] = {\n";
foreach(@vertices) {
    my @vertex = @{$_};
    print "\t{ F(" . $vertex[0]*$scale .
        "), F(" . $vertex[1]*$scale .
        "), F(" . $vertex[2]*$scale . ") }, \n";
}
print "};\n\n";

print "const init_vertex_t normals" . $suffix . "[] = {\n";
foreach(@normalsarr) {
        my @normal = @{$_};
        print "\t{ F(" . $normal[0]*1.0 .
                "), F(" . $normal[1]*1.0 .
                "), F(" . $normal[2]*1.0 . ") }, \n";
}
print "};\n\n";

print "const index_triangle_t faces" . $suffix . "[] = {\n";
foreach(@faces) {
    my @face = @{$_};
    print "\t{" . ($face[0] - 1 + $offset) . ", " .
                ($face[1] - 1 + $offset) . ", " .
                ($face[2] - 1 + $offset) . ", " .
                ($face[3] - 1 + $offset) . "},\n";
}
print "};\n";
