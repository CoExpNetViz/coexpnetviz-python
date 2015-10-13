
function delete_field(field) {
    for(i=field; i<NF; ++i) {
        $i = $(i+1)
    }
    NF--;
}

# From: http://stackoverflow.com/a/27158086/1031434
function ltrim(s) {
    sub(/^[ \t\r\n]+/, "", s);
    return s;
}
function rtrim(s) {
    sub(/[ \t\r\n]+$/, "", s);
    return s;
}
function trim(s) {
    return rtrim(ltrim(s))
}
