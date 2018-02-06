#include "cmyhtmlparser.h"
#include "assert/advanced_assert.h"

#include <assert.h>

CMyHtmlParser::CMyHtmlParser(size_t workerThreadCount)
{
	_myhtmlInstance = myhtml_create();
	if (workerThreadCount == 0)
		myhtml_init(_myhtmlInstance, MyHTML_OPTIONS_PARSE_MODE_SINGLE, 1, 0);
	else
		myhtml_init(_myhtmlInstance, MyHTML_OPTIONS_DEFAULT, workerThreadCount, 0);

	_knownEncodings = {
		{ "utf-8", MyENCODING_UTF_8 },
		{ "windows-1250", MyENCODING_WINDOWS_1250 },
		{ "windows-1251", MyENCODING_WINDOWS_1251 }
	};
}

CMyHtmlParser::~CMyHtmlParser()
{
	if (_myhtmlInstance)
		myhtml_destroy(_myhtmlInstance);
}

const std::vector<CMyHtmlParser::HtmlTag>& CMyHtmlParser::parse(const QByteArray& html)
{
	_tags.clear();
	_encoding.clear();

	_tree = myhtml_tree_create();
	assert_r(_tree);
	assert_r(myhtml_tree_init(_tree, _myhtmlInstance) == MyCORE_STATUS_OK);
	myhtml_callback_tree_node_insert_set(_tree, &CMyHtmlParser::callbackNodeInserted, this);

	assert_r(myhtml_parse(_tree, MyENCODING_UTF_8, html.data(), html.size()) == MyCORE_STATUS_OK);

	const auto actualEncoding = _knownEncodings.find(_encoding);
	if (actualEncoding != _knownEncodings.end() && actualEncoding->second != MyENCODING_UTF_8)
	{
		// Non-UTF-8 encoding detected, restarting parse
		_tags.clear();
		myhtml_tree_destroy(_tree);
		_tree = myhtml_tree_create();
		assert_r(_tree);
		assert_r(myhtml_tree_init(_tree, _myhtmlInstance) == MyCORE_STATUS_OK);
		myhtml_callback_tree_node_insert_set(_tree, &CMyHtmlParser::callbackNodeInserted, this);

		// Restarting parse with the correct encoding
		assert_r(myhtml_parse(_tree, actualEncoding->second, html.data(), html.size()) == MyCORE_STATUS_OK);
	}

	myhtml_tree_destroy(_tree);

	return _tags;
}

void CMyHtmlParser::callbackNodeInserted(myhtml_tree_t* /*tree*/, myhtml_tree_node_t* node)
{
	HtmlTag tag;
	tag.type = myhtml_node_tag_id(node);
	const char* text = myhtml_node_text(node, nullptr);
	if (text)
		tag.text = QString::fromUtf8(text);

	for (myhtml_tree_attr_t *attr = myhtml_node_attribute_first(node); attr != nullptr; attr = myhtml_attribute_next(attr))
	{
		HtmlTagAttribute attribute;

		const char *name = myhtml_attribute_key(attr, nullptr);
		if (name)
			attribute.name = QString::fromUtf8(name);

		const char *value = myhtml_attribute_value(attr, nullptr);
		if (value)
			attribute.value = QString::fromUtf8(value);

		assert(name || value);
		tag.attributes.push_back(attribute);

		if (tag.type == MyHTML_TAG_META && attribute.name == QLatin1String("charset"))
			_encoding = attribute.value.toLower();
	}

	_tags.push_back(tag);
}
