#pragma once

#include "myhtml/api.h"
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <vector>

class CMyHtmlParser
{
public:
	struct HtmlTagAttribute {
		QString name;
		QString value;
	};

	struct HtmlTag {
		inline QString attributeValue(const QString& attributeName) const {
			const auto it = std::find_if(attributes.cbegin(), attributes.cend(), [&attributeName](const HtmlTagAttribute& attr) {
				return attr.name == attributeName;
			});

			return it != attributes.cend() ? it->value : QString();
		}

		myhtml_tag_id_t type = MyHTML_TAG__UNDEF;
		QString text;
		std::vector<HtmlTagAttribute> attributes;
	};


	// 0 means no worker threads
	CMyHtmlParser();
	~CMyHtmlParser();

	const std::vector<HtmlTag>& parse(const QByteArray& html);

	const std::vector<HtmlTag>& result() const;

	QString documentEncodingName() const;

private:
	void callbackNodeInserted(myhtml_tree_t* tree, myhtml_tree_node_t* node);
	inline static void callbackNodeInserted(myhtml_tree_t* tree, myhtml_tree_node_t* node, void* ctx) {
		static_cast<CMyHtmlParser*>(ctx)->callbackNodeInserted(tree, node);
	}

private:
	std::vector<HtmlTag> _tags;
	myencoding_list _encoding = MyENCODING_NOT_DETERMINED;

	myhtml_t* _myhtmlInstance = nullptr;
	myhtml_tree_t* _tree = nullptr;
};
