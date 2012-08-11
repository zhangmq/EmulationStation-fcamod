#include "GuiTheme.h"
#include "../MathExp.h"
#include <iostream>
#include "GuiImage.h"
#include <boost/filesystem.hpp>

GuiTheme::GuiTheme(std::string path)
{
	if(!path.empty())
		readXML(path);
}

GuiTheme::~GuiTheme()
{
	deleteComponents();
}

void GuiTheme::deleteComponents()
{
	for(unsigned int i = 0; i < mComponentVector.size(); i++)
	{
		delete mComponentVector.at(i);
	}

	mComponentVector.clear();

	clearChildren();
}



void GuiTheme::readXML(std::string path)
{
	deleteComponents();

	mPath = path;

	if(path.empty())
		return;

	std::cout << "Loading theme \"" << path << "\"...\n";

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(path.c_str());

	if(!result)
	{
		std::cerr << "Error parsing theme \"" << path << "\"!\n";
		std::cerr << "	" << result.description() << "\n";
		return;
	}

	pugi::xml_node root = doc.child("theme");

	for(pugi::xml_node data = root.child("component"); data; data = data.next_sibling("component"))
	{
		createElement(data, this);
	}

	std::cout << "Finished parsing theme.\n";
}

GuiComponent* GuiTheme::createElement(pugi::xml_node data, GuiComponent* parent)
{
	std::string type = data.child("type").text().get();

	if(type == "image")
	{
		std::string path = expandPath(data.child("path").text().get());

		if(!boost::filesystem::exists(path))
		{
			std::cerr << "Error - theme image \"" << path << "\" does not exist.\n";
			return NULL;
		}

		std::string pos = data.child("pos").text().get();
		std::string dim = data.child("dim").text().get();

		//split position and dimension information
		size_t posSplit = pos.find(' ');
		std::string posX = pos.substr(0, posSplit);
		std::string posY = pos.substr(posSplit + 1, pos.length() - posSplit - 1);

		size_t dimSplit = dim.find(' ');
		std::string dimW = dim.substr(0, dimSplit);
		std::string dimH = dim.substr(dimSplit + 1, dim.length() - dimSplit - 1);

		std::cout << "image, x: " << posX << " y: " << posY << " w: " << dimW << " h: " << dimH << "\n";

		//resolve to pixels from percentages/variables
		int x = resolveExp(posX) * Renderer::getScreenWidth();
		int y = resolveExp(posY) * Renderer::getScreenHeight();
		int w = resolveExp(dimW) * Renderer::getScreenWidth();
		int h = resolveExp(dimH) * Renderer::getScreenHeight();

		std::cout << "w: " << w << "px, h: " << h << "px\n";

		GuiComponent* comp = new GuiImage(x, y, path, w, h, true);
		parent->addChild(comp);
		mComponentVector.push_back(comp);
		return comp;
	}


	std::cerr << "Type \"" << type << "\" unknown!\n";
	return NULL;
}

std::string GuiTheme::expandPath(std::string path)
{
	if(path[0] == '~')
		path = getenv("HOME") + path.substr(1, path.length() - 1);
	else if(path[0] == '.')
		path = boost::filesystem::path(mPath).parent_path().string() + path.substr(1, path.length() - 1);

	return path;
}

float GuiTheme::resolveExp(std::string str)
{
	MathExp exp;
	exp.setExpression(str);

	//set variables
	exp.setVariable("headerHeight", Renderer::getFontHeight(Renderer::LARGE) / Renderer::getScreenHeight());

	return exp.eval();
}