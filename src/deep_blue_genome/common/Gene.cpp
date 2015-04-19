// Author: Tim Diels <timdiels.m@gmail.com>

#include "Gene.h"
#include <deep_blue_genome/common/SpliceVariant.h>
#include <deep_blue_genome/common/OrthologGroup.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

Gene::Gene()
:	gene_collection(nullptr), ortholog_group(nullptr)
{
}

Gene::Gene(const std::string& name, GeneCollection& gene_collection, OrthologGroup& ortholog_group)
:	name(name), gene_collection(&gene_collection), ortholog_group(nullptr)
{
	ortholog_group.add(*this);
}

GeneCollection& Gene::get_gene_collection() const {
	return *gene_collection;
}

std::string Gene::get_name() const {
	return name;
}

OrthologGroup& Gene::get_ortholog_group() const {
	return *ortholog_group;
}

SpliceVariant& Gene::get_splice_variant(SpliceVariantId id) {
	auto match_id = [id](const unique_ptr<SpliceVariant>& variant) {
		return variant->get_splice_variant_id() == id;
	};

	auto it = find_if(splice_variants.begin(), splice_variants.end(), match_id);
	if (it == splice_variants.end()) {
		// Add if does not exist yet
		splice_variants.emplace_back(make_unique<SpliceVariant>(*this, id));
		return *splice_variants.back();
	}
	return **it;
}

Gene& Gene::get_gene() {
	return *this;
}

Gene& Gene::as_gene() {
	return *this;
}

void Gene::set_ortholog_group(OrthologGroup& group) {
	ortholog_group = &group;
}

void Gene::print(std::ostream& str) const {
	str << "gene '" << name << "'";
}

} // end namespace
