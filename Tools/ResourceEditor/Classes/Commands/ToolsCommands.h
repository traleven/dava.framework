#ifndef __TOOLS_COMMANDS_H__
#define __TOOLS_COMMANDS_H__

#include "Command.h"
#include "../Constants.h"

class CommandMaterials: public Command
{
public:	
	CommandMaterials();

protected:	
    
    virtual void Execute();
};


class CommandTextureConverter: public Command
{
public:	
	CommandTextureConverter();
    
protected:	
    
    virtual void Execute();
};

class CommandHeightmapEditor: public Command
{
public:	
	CommandHeightmapEditor();
    
protected:	
    
    virtual void Execute();
};

class CommandTilemapEditor: public Command
{
public:	
	CommandTilemapEditor();
    
protected:	
    
    virtual void Execute();
};




#endif // #ifndef __TOOLS_COMMANDS_H__