/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#import "../include/imebra/objectivec/imebra_strings.h"
#import <Foundation/NSString.h>

namespace imebra
{

std::string NSStringToString ( NSString* str )
{
    return std::string([str UTF8String]);
}

NSString* stringToNSString ( const std::string& str )
{
    return [ [ NSString alloc] initWithUTF8String: str.c_str()];
}

} // namespace imebra
