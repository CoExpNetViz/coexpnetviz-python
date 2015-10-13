@include csv-parser.awk

function csv_parse_record_(separator, enclosure,    num_fields, csv, i) {
    num_fields = csv_parse_record($0, separator, enclosure, csv);
    if (num_fields < 0) {
        print "CSV parse error on line " FNR;
        exit(1);
    }

    # Now overwrite regular fields with csv fields
    NF = num_fields;
    for (i=0; i<num_fields; i++) {
        $(i+1) = csv[i];
    }
}

