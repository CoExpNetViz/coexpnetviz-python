// Author: Tim Diels <timdiels.m@gmail.com>

#include "OrthologGroupInfos.h"
#include <deep_blue_genome/common/Gene.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

OrthologGroupInfos::OrthologGroupInfos(GeneCollections gene_collections)
:	gene_collections(std::move(gene_collections))
{
}

OrthologGroupInfo* OrthologGroupInfos::get(const Gene& gene) {
	auto group = gene.get_ortholog_group();

	if (!group) {
		return nullptr;
	}

	auto it = groups.find(group);
	if (it == groups.end()) {
		// make group
		auto p = groups.emplace(piecewise_construct,
				forward_as_tuple(group),
				forward_as_tuple(*group, gene_collections)
		);
		return &p.first->second;
	}
	else {
		return &it->second;
	}
}

}} // end namespace
