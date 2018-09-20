import json
from mako.template import Template

template_Object = '''
#include <FishEngine/Serialization/Serialize.hpp>
#include <FishEngine/Serialization/Archive.hpp>
#include <FishEngine/FishEngine2.hpp>

using namespace FishEngine;
using namespace FishEditor;
using namespace FishEditor::Animation;


% for c in ClassInfo:
// ${c['name']}
void ${c['scope_prefix']}::${c['name']}::Deserialize(InputArchive& archive)
{
% if c['base_class'] is not None:
	${c['base_class']}::Deserialize(archive);
% endif
% for member in c['members']:
	archive.AddNVP("${member['name']}", this->${member['name']});
% endfor
}

void ${c['scope_prefix']}::${c['name']}::Serialize(OutputArchive& archive) const
{
% if c['base_class'] is not None:
	${c['base_class']}::Serialize(archive);
% endif
% for member in c['members']:
	archive.AddNVP("${member['name']}", this->${member['name']});
% endfor
}


% endfor
'''

with open('components.json', 'r') as f:
	components = json.load(f)

s = Template(template_Object.strip()).render(ClassInfo=components)
print(s)

def Gen(comp):
	pass