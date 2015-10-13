# orcae functional annotations file:
# - csv
# - multiple annotations per gene

@include csv.awk
@include util.awk

BEGIN {
    OFS="\t"
}

{
    csv_parse_record_(";", "\"");
    gene=trim($2)
    annotation=trim($5)

    # remove illegal characters from annotation
    gsub(/\t\r\n/, " ", annotation)

    # skip lines with an empty field
    if (gene == "" || annotation == "") {
        next
    }

    # merge multiple annotations to same gene
    if (annotations[gene])
        annotations[gene] = annotations[gene] ". " annotation;
    else
        annotations[gene] = annotation
}

END {
    for (gene in annotations)
        print gene, annotations[gene];
}
