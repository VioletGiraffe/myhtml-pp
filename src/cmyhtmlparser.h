#pragma once

#include "myhtml/api.h"
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
RESTORE_COMPILER_WARNINGS

#include <vector>

class CMyHtmlParser
{
public:
	struct HtmlTagAttribute {
		QString name;
		QString value;
	};

	struct HtmlTag {
		myhtml_tag_id_t type = MyHTML_TAG__UNDEF;
		QString text;
		std::vector<HtmlTagAttribute> attributes;
	};


	// 0 means no worker threads
	explicit CMyHtmlParser(size_t workerThreadCount = 0);
	~CMyHtmlParser();

	void parse(const QByteArray& html);
	inline const std::vector<HtmlTag>& result() const {
		return _tags;
	}

private:
	void callbackNodeInserted(myhtml_tree_t* tree, myhtml_tree_node_t* node);
	inline static void callbackNodeInserted(myhtml_tree_t* tree, myhtml_tree_node_t* node, void* ctx) {
		static_cast<CMyHtmlParser*>(ctx)->callbackNodeInserted(tree, node);
	}

private:
	std::vector<HtmlTag> _tags;

	myhtml_t* _myhtmlInstance = nullptr;
	myhtml_tree_t* _tree = nullptr;
};
