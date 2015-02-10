-- Create deep blue genome database structure

-- Note: MySQL doesn't support CHECK constraints (it does parse them though...)

-- Drop all
drop table if exists gene_collection, gene, gene_mapping, expression_matrix, expression_matrix_row, clustering, cluster, cluster_item;
drop table expression_matrix_cell, orthologs, genome;

-- Set collation
ALTER DATABASE db_tidie_deep_blue_genome
DEFAULT COLLATE utf8_general_ci;


-- Use MyISAM (no transactions, no foreign key constraint checks, better performance)
SET storage_engine = MyISAM;

-- General application settings 
create table settings (
	id INT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARCHAR(250) NOT NULL COMMENT 'Name of setting',
	value VARCHAR(500) NOT NULL COMMENT 'Value of setting',
	PRIMARY KEY (id) 
);

-- A collection of genes from a single source with a common naming scheme
create table gene_collection (
	id INT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARCHAR(100) NOT NULL COMMENT 'Name of collection, e.g. RAP db',
	species VARCHAR(100) NOT NULL COMMENT 'Name of species',
	gene_format_match VARCHAR(500) NOT NULL COMMENT 'Genes whose name match this perl regex are part of the collection',
	gene_format_replace VARCHAR(500) NOT NULL COMMENT 'Formats a gene name matched with gene_formatter_match to its user-presentable canonical form. See http://www.boost.org/doc/libs/1_57_0/libs/regex/doc/html/boost_regex/format/boost_format_syntax.html',
	PRIMARY KEY (id),
	FOREIGN KEY (genome_id) REFERENCES genome(id)
);

create table gene (
	id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARCHAR(256) NOT NULL UNIQUE COMMENT 'Gene name formatted with gene_collection.gene_formatter', -- This name doesn't include a splice variant number (as those are considered variants of the same gene)
	gene_collection_id INT UNSIGNED NOT NULL,
	functional_annotation VARCHAR(1000),
	ortholog_group BIGINT UNSIGNED COMMENT 'genes of the same ortholog_group are each other's orthologs',
	PRIMARY KEY (id),
	FOREIGN KEY (gene_collection_id) REFERENCES gene_collection(id)
);
-- TODO sorted index on ortholog_group

-- Genes from a different gene collection with a similar locus and sequence.
-- This is a symmetric relation
-- E.g. MSU - RAP mapping
create table gene_mapping (
	gene1_id BIGINT UNSIGNED NOT NULL,
	gene2_id BIGINT UNSIGNED NOT NULL UNIQUE, -- Note: gene1.gene_collection != gene2.gene_collection 
	PRIMARY KEY (gene1_id),
	FOREIGN KEY (gene1_id) REFERENCES gene(id),
	FOREIGN KEY (gene2_id) REFERENCES gene(id)
);
-- TODO put index on gene2_id as well (gene1 has one via primary key)

-- Gene expression matrices
create table expression_matrix (
	id INT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARCHAR(250) NOT NULL,
	gene_collection_id INT UNSIGNED NOT NULL,
	PRIMARY KEY (id),
	FOREIGN KEY (gene_collection_id) REFERENCES gene_collection(id)
);

create table expression_matrix_row (
	matrix_id INT UNSIGNED NOT NULL,
	row INT UNSIGNED NOT NULL COMMENT 'Row index, zero-based',
	gene_id BIGINT UNSIGNED NOT NULL,
	PRIMARY KEY (matrix_id, row),
	FOREIGN KEY (matrix_id) REFERENCES expression_matrix(id)
);

-- Clustering of genes of one gene collection
create table clustering (
	id INT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARCHAR(250) NOT NULL,
	gene_collection_id INT UNSIGNED NOT NULL,
	expression_matrix_id INT UNSIGNED COMMENT 'Some clusterings can only be used with a specific matrix',
	PRIMARY KEY (id),
	FOREIGN KEY (gene_collection_id) REFERENCES gene_collection(id)
	UNIQUE (name, gene_collection_id, expression_matrix_id)
);

create table cluster (
	id INT UNSIGNED NOT NULL AUTO_INCREMENT,
	clustering_id INT UNSIGNED NOT NULL,
	name VARCHAR(100) NOT NULL,
	PRIMARY KEY (id),
	FOREIGN KEY (clustering_id) REFERENCES clustering(id)
);

create table cluster_item (
	cluster_id INT UNSIGNED NOT NULL,
	gene_id BIGINT UNSIGNED NOT NULL,
	PRIMARY KEY (cluster_id, gene_id),
	FOREIGN KEY (cluster_id) REFERENCES cluster(id),
	FOREIGN KEY (gene_id) REFERENCES gene(id)
);

