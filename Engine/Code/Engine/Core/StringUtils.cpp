#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}



Strings SplitStringOnDelimiter(std::string const& originalString, char delimiterToSplitOn)
{
	std::vector<std::string> result;
	std::string temp;
	for (int i = 0; i < (int)originalString.size(); i++) 
	{
		if (originalString[i] == delimiterToSplitOn) 
		{
			result.push_back(temp);
			temp.clear(); 
		}
		else 
		{
			temp += originalString[i];
		}
	}

	result.push_back(temp);

	return result;
}

Strings SplitStringOnDelimiterIgnoreSpace(std::string const& originalString, char delimiterToSplitOn)
{
	std::vector<std::string> result;
	std::string temp;
	for (int i = 0; i < (int)originalString.size(); i++)
	{
		if (originalString[i] == delimiterToSplitOn)
		{
			result.push_back(temp);
			temp.clear();
		}
		else
		{
			if (originalString[i] != ' ')
			{
				temp += originalString[i];
			}
		}
	}

	result.push_back(temp);

	return result;
}

Strings SplitStringOnDelimiterIgnoreChangeLine(std::string const& originalString, char delimiterToSplitOn)
{
	std::vector<std::string> result;
	std::string temp;
	for (int i = 0; i < (int)originalString.size(); i++)
	{
		if (originalString[i] == delimiterToSplitOn)
		{
			result.push_back(temp);
			temp.clear();
		}
		else
		{
			if (originalString[i] != '\n')
			{
				temp += originalString[i];
			}
		}
	}

	result.push_back(temp);

	return result;
}






