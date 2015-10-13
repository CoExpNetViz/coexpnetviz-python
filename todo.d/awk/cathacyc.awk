@include csv.awk
@include util.awk
BEGIN {
    OFS="\t"
}
FNR>1 {
    csv_parse_record_(",", "\"");
    sub("^ID=", "", $1);
    delete_field(2);
    print;
}
