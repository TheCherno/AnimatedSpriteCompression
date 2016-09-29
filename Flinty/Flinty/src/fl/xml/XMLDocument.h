#pragma once

#include "fl/Common.h"
#include "fl/String.h"
#include "rapidxml/rapidxml.hpp"

namespace fl {

	struct FL_API XMLAttribute
	{
		String name;
		String value;
	};

	struct FL_API XMLNode
	{
		static const XMLNode* NullNode;

		String name;
		String value;
		std::vector<XMLAttribute> attributes;
		std::vector<XMLNode> children;
		const XMLNode& parent;

		XMLNode(const String& name, const XMLNode& parent = *NullNode)
			: name(name), parent(parent) {}
		XMLNode(const String& name, const String& value, const XMLNode& parent = *NullNode)
			: name(name), value(value), parent(parent) {}

		XMLAttribute* FindAttribute(const String& name)
		{
			for (XMLAttribute& attribute : attributes)
			{
				if (attribute.name == name)
					return &attribute;
			}
			return nullptr;
		}

		XMLNode* FindChild(const String& name)
		{
			for (XMLNode& child : children)
			{
				if (child.name == name)
					return &child;
			}
			return nullptr;
		}
	};

	class FL_API XMLDocument
	{
	private:
		String m_Filepath;
		XMLNode* m_RootNode;
	public:
		XMLDocument(const String& filepath);
		~XMLDocument();

		inline XMLNode& GetRootNode() const { return *m_RootNode; }
		XMLNode* FindNode(const String& name);
	private:
		void ParseAllAttributes(rapidxml::xml_node<>* node, std::vector<XMLAttribute>& attributes);
		void ParseNodeChildren(rapidxml::xml_node<>* node, XMLNode& base);

		XMLNode* FindChild(XMLNode& node, const String& name);
	};

}