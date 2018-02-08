#include "cmyhtmlparser.h"
#include "assert/advanced_assert.h"

CMyHtmlParser::CMyHtmlParser()
{
	_myhtmlInstance = myhtml_create();
	myhtml_init(_myhtmlInstance, MyHTML_OPTIONS_PARSE_MODE_SINGLE, 1, 0);
}

CMyHtmlParser::~CMyHtmlParser()
{
	if (_myhtmlInstance)
		myhtml_destroy(_myhtmlInstance);
}

const std::vector<CMyHtmlParser::HtmlTag>& CMyHtmlParser::parse(const QByteArray& html)
{
	_tags.clear();

	_tree = myhtml_tree_create();
	assert_r(_tree);
	assert_r(myhtml_tree_init(_tree, _myhtmlInstance) == MyCORE_STATUS_OK);
	myhtml_callback_tree_node_insert_set(_tree, &CMyHtmlParser::callbackNodeInserted, this);

	/*
	* From Specification:
	*
	* The authoring conformance requirements for character encoding declarations limit them to only
	* appearing in the first 1024 bytes. User agents are therefore encouraged to use the prescan
	* algorithm below (as invoked by these steps) on the first 1024 bytes, but not to stall beyond that.
	*/
	_encoding = myencoding_prescan_stream_to_determine_encoding(html.data(), std::min(html.size(), 1024));
	if (_encoding == MyENCODING_NOT_DETERMINED)
	{
		assert_unconditional_r("Failed to determine data encoding");
		_encoding = MyENCODING_UTF_8;
	}

	assert_r(myhtml_parse(_tree, _encoding, html.data(), html.size()) == MyCORE_STATUS_OK);
	myhtml_tree_destroy(_tree);

	return _tags;
}

const std::vector<CMyHtmlParser::HtmlTag>& CMyHtmlParser::result() const
{
	return _tags;
}

QString CMyHtmlParser::documentEncodingName() const
{
	return myencoding_name_by_id(_encoding, nullptr);
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
	}

	_tags.push_back(tag);
}
