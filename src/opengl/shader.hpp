#pragma once
#include <string>
class shader
{
  private:
	void init();
	void clear();
	bool inited=false;
	std::string loadFile(std::string file);
  public:
	~shader();
	uint programId;
	void loadFromFile(std::string file,unsigned int mode);
	void load(std::string &source,unsigned int mode);
	int getUniform(std::string str);
	int getAttribute(std::string str);
	void use();
	static void unuse();
};
