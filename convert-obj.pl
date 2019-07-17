#!/usr/bin/perl

use warnings;
use strict;

my $scale = 0.6;
my $offset = 0;
my $suffix = $ARGV[1];

my @vertices;
my @faces;

my $material = 0;
my $lastmaterial = 0;
my %materials = ();
my %vert_texcoords = ();
my @texcoords = ();

open(my $in_file, "<", $ARGV[0]);
open(my $out_file_h, ">", $ARGV[3]);
open(my $out_file_c, ">", $ARGV[2]);

while(<$in_file>) {
    chomp;
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

    if( $_ =~ /vt ([^ ]+) ([^ ]+)/ ) {
        push @texcoords, [$1, $2];
    }
    
    # Assume normals are per face, no whatever calculate normals later
    if( $_ =~ /f ([0-9]+)\/([0-9]*)\/([0-9]+) ([0-9]+)\/([0-9]*)\/([0-9]+) ([0-9]+)\/[0-9]*\/([0-9]+)/ ) {
        push @faces, [$1, $4, $7, $3];
        
        # Likely reasonable assumption: Texcoords are unique per vertex (WRONG)
        if( length($2) != 0 ) {
            $vert_texcoords{$1} = $2;
            $vert_texcoords{$4} = $5;
            $vert_texcoords{$7} = $8;
        }
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

print $out_file_h "#ifndef " . uc($ARGV[1]) . "_H\n";
print $out_file_h "#define " . uc($ARGV[1]) . "_H\n\n";
print $out_file_h "#include \"Rasterize.h\"\n\n";
print $out_file_h "#define numVertices$suffix " . scalar @vertices . "\n";
print $out_file_h "#define numNormals$suffix " . scalar @normalsarr . "\n";
print $out_file_h "#define numFaces$suffix " . scalar @faces . "\n\n";
print $out_file_h "extern const init_vertex_t vertices" . $suffix . "[];\n";
print $out_file_h "extern const init_vertex_t normals" . $suffix . "[];\n";
print $out_file_h "extern const index_triangle_t faces" . $suffix . "[];\n";
print $out_file_h "extern const vec2_t texcoords" . $suffix . "[];\n\n";
print $out_file_h "#endif\n\n";

print $out_file_c "#include \"Rasterize.h\"\n\n";

print $out_file_c "const init_vertex_t vertices" . $suffix . "[] = {\n";
foreach(@vertices) {
    my @vertex = @{$_};
    print $out_file_c "\t{ F(" . $vertex[0]*$scale .
        "), F(" . $vertex[1]*$scale .
        "), F(" . $vertex[2]*$scale . ") }, \n";
}
print $out_file_c "};\n\n";

print $out_file_c "const init_vertex_t normals" . $suffix . "[] = {\n";
foreach(@normalsarr) {
        my @normal = @{$_};
        print $out_file_c "\t{ F(" . $normal[0]*1.0 .
                "), F(" . $normal[1]*1.0 .
                "), F(" . $normal[2]*1.0 . ") }, \n";
}
print $out_file_c "};\n\n";

print $out_file_c "const index_triangle_t faces" . $suffix . "[] = {\n";
foreach(@faces) {
    my @face = @{$_};
    print $out_file_c "\t{" . ($face[0] - 1 + $offset) . ", " .
                ($face[1] - 1 + $offset) . ", " .
                ($face[2] - 1 + $offset) . ", " .
                ($face[3] - 1 + $offset) . "},\n";
}
print $out_file_c "};\n";

print $out_file_c "const vec2_t texcoords" . $suffix . "[] = {\n";
for(my $i = 0; $i < scalar @vertices; $i++) {
    my $texcoord_idx = $vert_texcoords{$i + 1} - 1;
    my @texcoord = @{$texcoords[$texcoord_idx]};
    print $out_file_c "\t{ F(" . $texcoord[0] . "), F(" . $texcoord[1] . ") }, \n";
}
print $out_file_c "};\n";
