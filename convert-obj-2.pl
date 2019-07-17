#!/usr/bin/perl

use warnings;
use strict;

my $scale = 0.6;
my $offset = 0;
my $suffix = $ARGV[1];

my @vertices;
my @faces;

my @normals;
my @tangents;

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
    
    # everything is per vertex now, and MUST be specified
    # tangent indices are the same as normal indices
    if( $_ =~ /f ([0-9]+)\/([0-9]*)\/([0-9]+) ([0-9]+)\/([0-9]*)\/([0-9]+) ([0-9]+)\/([0-9]*)\/([0-9]+)/ ) {
        push @faces, [$1, $4, $7, $3, $6, $9, $2, $5, $8];
    }
    
    if( $_ =~ /vn ([^ ]+) ([^ ]+) ([^ ]+)/ ) {
        push @normals, [$1, $2, $3];
    }
    
    if( $_ =~ /vg ([^ ]+) ([^ ]+) ([^ ]+)/ ) {
        push @tangents, [$1, $2, $3];
    }    
}

print $out_file_h "#ifndef " . uc($ARGV[1]) . "_H\n";
print $out_file_h "#define " . uc($ARGV[1]) . "_H\n\n";
print $out_file_h "#include \"Rasterize.h\"\n\n";
print $out_file_h "#define numVertices$suffix " . scalar @vertices . "\n";
print $out_file_h "#define numNormals$suffix " . scalar @normals . "\n";
print $out_file_h "#define numFaces$suffix " . scalar @faces . "\n\n";
print $out_file_h "extern const init_vertex_t vertices" . $suffix . "[];\n";
print $out_file_h "extern const init_vertex_t normals" . $suffix . "[];\n";
print $out_file_h "extern const index_trianglepv_t faces" . $suffix . "[];\n";
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
foreach(@normals) {
        my @normal = @{$_};
        print $out_file_c "\t{ F(" . $normal[0]*1.0 .
                "), F(" . $normal[1]*1.0 .
                "), F(" . $normal[2]*1.0 . ") }, \n";
}
print $out_file_c "};\n\n";

print $out_file_c "const init_vertex_t tangents" . $suffix . "[] = {\n";
foreach(@tangents) {
        my @tangent = @{$_};
        print $out_file_c "\t{ F(" . $tangent[0]*1.0 .
                "), F(" . $tangent[1]*1.0 .
                "), F(" . $tangent[2]*1.0 . ") }, \n";
}
print $out_file_c "};\n\n";

print $out_file_c "const index_trianglepv_t faces" . $suffix . "[] = {\n";
foreach(@faces) {
    my @face = @{$_};
    print $out_file_c "\t{" . ($face[0] - 1 + $offset) . ", " .
                ($face[1] - 1 + $offset) . ", " .
                ($face[2] - 1 + $offset) . ", " .
                ($face[3] - 1 + $offset) . ", " .
                ($face[4] - 1 + $offset) . ", " .
                ($face[5] - 1 + $offset) . ", " .
                ($face[6] - 1 + $offset) . ", " .
                ($face[7] - 1 + $offset) . ", " .
                ($face[8] - 1 + $offset) . "},\n";
}
print $out_file_c "};\n";

print $out_file_c "const vec2_t texcoords" . $suffix . "[] = {\n";
foreach(@texcoords) {
    my @texcoord = @{$_};
    print $out_file_c "\t{ F(" . $texcoord[0] . "), F(" . $texcoord[1] . ") }, \n";
}
print $out_file_c "};\n\n";
