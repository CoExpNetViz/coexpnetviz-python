// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneFamilyId.h"

using namespace std;
using namespace boost;

namespace DEEP_BLUE_GENOME {

GeneFamilyId::GeneFamilyId()
{
}

GeneFamilyId::GeneFamilyId(std::string source, std::string id)
:	source(source), id(id)
{
}

std::string GeneFamilyId::get_source() const {
	return source;
}

std::string GeneFamilyId::get_id() const {
	return id;
}

bool GeneFamilyId::operator==(const GeneFamilyId& o) const {
	return id == o.id && source == o.source;
}

bool GeneFamilyId::operator<(const GeneFamilyId& o) const {
	if (source < o.source)
		return true;
	return id < o.id;
}

std::ostream& operator<<(std::ostream& out, const GeneFamilyId& id) {
	out << "(" << id.get_source() << ", " << id.get_id() << ")";
	return out;
}

} // end namespace
