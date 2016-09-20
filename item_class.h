#pragma once

#include <set>
#include <vector>
#include <map>

struct MagicItem;

//todo: reimplement so just has one vector<record>
struct ItemClass
{
	virtual ~ItemClass();

	std::string get_identifier(int type);

	bool is_discovered(int type);
	void discover(int type);

	std::string get_guess(int type);
	void set_guess(int type, const std::string& guess);
	//call_it2: Call an object something after use.
	void call_it2(int type);

	std::vector<MagicItem> m_magic_props;

protected:
	std::vector<std::string> m_identifier;
private:
	std::set<int> m_discoveries;
	std::map<int, std::string> m_guesses;
};

struct ScrollInfo : public ItemClass
{
	ScrollInfo();
};

struct PotionInfo : public ItemClass
{
	PotionInfo();
};

struct StickInfo : public ItemClass
{
	StickInfo();
	bool is_staff(int which) const;
	std::vector<std::string> m_type;
};

struct RingInfo : public ItemClass
{
	RingInfo();
};
