#ifndef _SLD_COMPARE_H_
#define _SLD_COMPARE_H_

#include "SldError.h"
#include "SldDefines.h"
#include "SldPlatform.h"
#include "SldTypes.h"
#include "SldSDCReadMy.h"
#include "SldSymbolsTable.h"
// TODO: get rid of different levels of nesting in include
//#include "../DataReader.h"

// Maximum length of a string with a parameter value
static const UInt32 MetaParamMaxValSize = 1024;

// Comparison table disassembled into main parts
typedef struct TCompareTableSplit
{
	// Pointer to the header of the comparison table
	CMPHeaderType									*Header;

	// Pointer to array of weights of simple characters (one weight per character)
	CMPSimpleType									*Simple;

	// Pointer to an array of complex characters
	CMPComplexType									*Complex;

	// Pointer to a delimiter character array
	CMPDelimiterType								*Delimiter;

	// Pointer to an array of native characters of the language
	CMPNativeType									*Native;

	// Pointer to array of partial delimiters
	CMPHalfDelimiterType							*HalfDelimiter;

	// Pointer to the header of the uppercase / lowercase mapping table
	CMPSymbolPairTableHeader						*HeaderPairSymbols;

	// Pointer to an array of uppercase / lowercase match pairs for the given language
	CMPSymbolPair									*NativePair;

	// Pointer to an array of uppercase and lowercase character pairs for all
	// the characters we know
	CMPSymbolPair									*CommonPair;

	// Pointer to an array of uppercase and lowercase character pairs for
	// characters that appear in the wordlists of this dictionary
	CMPSymbolPair									*DictionaryPair;

	// Character mass table
	sld2::Array<UInt16, MAX_UINT16_VALUE>			SimpleMassTable;

	// Properties table for the most commonly used symbols
	sld2::Array<UInt8, CMP_MOST_USAGE_SYMBOL_RANGE>	MostUsageCharTable;

	// Array of characters sorted by mass
	sld2::DynArray<Int16>							SortedMass;

	// Size of the whole table
	UInt32											TableSize;
}TCompareTableSplit;

// Flag mask - defines what it is - the weight of the symbol or the index of the complex symbol table.
#define CMP_MASK_OF_INDEX_FLAG		(0x8000)
// Index mask - in case it is an index, removes the flag, leaving a clean index.
#define CMP_MASK_OF_INDEX			(0x7fff)
// Ignore character identifier.
#define CMP_IGNORE_SYMBOL			(0)
// The identifier of the symbol that was not found.
#define CMP_NOT_FOUND_SYMBOL		(0xffff)

// The code of the delimiter character in the wordlist (used in Chinese).
#define CMP_DELIMITER				(9)

// The default character to replace those characters that are not found.
#define CMP_DEFAULT_CHAR			(0x98)

// Character mass for null characters (in GetStrOfMass ())
#define CMP_MASS_ZERO				(0x7A00)
// Character mass for delimiters (in GetStrOfMass ())
#define CMP_MASS_DELIMITER			(0x7A01)
// Virtual zero mass
#define CMP_MASS_ZERO_DIGIT			(0x7A10)

// Character for escaping spec. characters in the search query
#define CMP_QUERY_SPECIAL_SYMBOL_ESCAPE_CHAR	'%'
// Specialist. the character in the search query is the AND operation
#define CMP_QUERY_SPECIAL_SYMBOL_AND			'&'
// Specialist. a character in a search query is an OR operation
#define CMP_QUERY_SPECIAL_SYMBOL_OR				'|'
// Specialist. character in a search query - NOT operation
#define CMP_QUERY_SPECIAL_SYMBOL_NOT			'!'
// Specialist. the character in the search term is an open parenthesis
#define CMP_QUERY_SPECIAL_SYMBOL_OPEN_BR		'('
// Specialist. a character in a search term is a closing parenthesis
#define CMP_QUERY_SPECIAL_SYMBOL_CLOSE_BR		')'
// Specialist. a character in a search query - a sequence of any characters
#define CMP_QUERY_SPECIAL_SYMBOL_ANY_CHARS		'*'
// Specialist. character in a string of masses - a sequence of any characters
#define CMP_MASS_SPECIAL_SYMBOL_ANY_CHARS		(0x7B01)
// Specialist. character in a search query - any character
#define CMP_QUERY_SPECIAL_SYMBOL_ONE_CHAR		'?'
// Specialist. character in the mass string - any character
#define CMP_MASS_SPECIAL_SYMBOL_ONE_CHAR		(0x7B02)

// Small version of the comparison table.
#define CMP_VERSION_1		1
// A version of the comparison table that has a direct mapping of characters to their weights.
#define CMP_VERSION_2		2

// Macro to determine if the end of a line has been reached
#define CMP_IS_EOL(header, str)		(*(str)==0 || ((header)->EOL==*(str)))

// Element number calculation
#define TIO(X, Y)	X][Y

// Determining the smallest of 3 characters
#define sldMin3(i1, i2, i3)			( sldMin2((i1), sldMin2((i2), (i3)))  )
// Determining the smallest of 2 characters
#define sldMin2(i1, i2)				( (i1)<(i2) ? (i1) : (i2))

// Checks if a character is a number
#define CMP_IS_DIGIT(ch)			( ((ch) > 0x2F) && ((ch) < 0x3A) ? 1 : 0)
// Returns the virtual mass of a character
#define CMP_VIRTUAL_DIGIT_MASS(ch)	( CMP_MASS_ZERO_DIGIT + (ch) - 0x30 )

// template functions for working with strings
namespace sld2 {

/**
 * Determines the length of a string
 *
 * @param[in]  aStr - pointer to string
 *
 * @return the length of the string, or 0 if a null pointer to a string is passed
 */
template <typename Char>
UInt32 StrLen(const Char *aStr)
{
	return aStr ? char_traits<Char>::length(aStr) : 0;
}

/**
 * Binary comparison of 2 strings
 *
 * @param[in]  aStr1 - pointer to the first of the compared strings
 * @param[in]  aStr2 - pointer to the second of the compared strings
 *
 * @return   0 - strings are equal
 *          >0 - the first line is greater than the second
 *          <0 - the first line is less than the second
 */
template <typename Char>
Int32 StrCmp(const Char *aStr1, const Char *aStr2)
{
	if (!aStr1 || !aStr2)
		return 0;

	for (; *aStr1 == *aStr2; aStr1++, aStr2++)
	{
		if (*aStr1 == 0)
			return 0;
	}
	return char_traits<Char>::cmp(*aStr1, *aStr2);
}

/**
 * Compare 2 strings case sensitive with the maximum number of characters to compare
 *
 * @param[in]  aStr1  - pointer to the first of the compared strings
 * @param[in]  aStr2  - pointer to the second of the compared strings
 * @param[in]  aCount - maximum number of characters to compare
 *
 * @return   0 - strings are equal
 *          >0 - the first line is greater than the second
 *          <0 - the first line is less than the second
 */
template <typename Char>
Int32 StrNCmp(const Char *aStr1, const Char *aStr2, UInt32 aCount)
{
	if (!aStr1 || !aStr2)
		return 0;

	for (; aCount; aStr1++, aStr2++, --aCount)
	{
		Int32 r = sld2::char_traits<Char>::cmp(*aStr1, *aStr2);
		if (r != 0)
			return r;
		if (*aStr1 == 0)
			break;
	}
	return 0;
}

/**
 * Returns whether 2 strings are equal
 *
 * @param[in]  aStr1 - pointer to the first of the compared strings
 * @param[in]  aStr2 - pointer to the second of the compared strings
 */
template <typename Char>
bool StrEqual(const Char *aStr1, const Char *aStr2)
{
	return StrCmp(aStr1, aStr2) == 0;
}

/**
 * Returns whether 2 strings are equal
 *
 * @param[in]  aStr1  - pointer to the first of the compared strings
 * @param[in]  aStr2  - pointer to the second of the compared strings
 * @param[in]  aCount - maximum number of characters to compare
 */
template <typename Char>
bool StrEqual(const Char *aStr1, const Char *aStr2, UInt32 aCount)
{
	return StrNCmp(aStr1, aStr2, aCount) == 0;
}

/**
 * Copies a string
 *
 * @param[out]  aDest   - pointer to the buffer where we will copy
 * @param[in]   aSource - pointer to the string to be copied
 *
 * @return number of characters copied (excluding null-terminator)
 */
template <typename Char>
UInt32 StrCopy(Char* aDest, const Char* aSource)
{
	if (!aDest || !aSource)
		return 0;

	UInt32 count = 0;
	for (; *aSource; aSource++, aDest++, count++)
		*aDest = *aSource;
	*aDest = 0;

	return count;
}

/**
 * Copies a string of length with the maximum number of characters to copy
 *
 * @param[out]  aDest   - pointer to the buffer where we will copy
 * @param[in]   aSource - pointer to the string to be copied
 * @param[in]   aCount  - maximum number of characters to copy
 *
 * @return number of characters copied (excluding null-terminator)
 */
template <typename Char>
UInt32 StrNCopy(Char* aDest, const Char* aSource, UInt32 aCount)
{
	if (!aDest || !aSource)
		return 0;

	UInt32 count = 0;
	for (; *aSource && count < aCount; aSource++, aDest++, count++)
		*aDest = *aSource;
	if (count < aCount)
		*aDest = 0;

	return count;
}

/**
 * Searches for the specified character in a string
 *
 * @param[in]  aStr - pointer to string
 * @param[in]  aChr - symbol
 *
 * @return if a character is found, then a pointer to its position in the string, otherwise nullptr
 */
template <typename Char>
const Char* StrChr(const Char *aStr, Char aChr)
{
	if (!aStr)
		return nullptr;

	for (; *aStr != aChr; aStr++)
	{
		if (*aStr == 0)
			return nullptr;
	}
	return aStr;
}
template <typename Char>
Char* StrChr(Char *aStr, Char aChr)
{
	return (Char*)StrChr((const Char*)aStr, aChr);
}

/**
 * Searches for the specified substring in a string
 *
 * @param[in]  aSource - the string in which the search occurs
 * @param[in]  aStr    - the substring we are looking for
 *
 * @return  pointer to the first occurrence of the desired substring in the
 *          string, or nullptr if the substring was not found.
 *          If aStr == nullptr returns aSource
 */
template <typename Char>
const Char* StrStr(const Char *aSource, const Char *const aStr)
{
	if (!aSource || !aStr)
		return aSource;

	if (*aStr == 0)
		return aSource;

	// the outer loop finds the first character of aStr in aSource
	// the inner one compares the rest of the substring on match
	for (; *aSource; aSource++)
	{
		if (*aSource != *aStr)
			continue;

		const Char *substring = aStr;
		for (const Char *string = aSource; ; string++, substring++)
		{
			if (*substring == 0)
				return aSource;

			if (*string != *substring)
				break;
		}
	}
	return nullptr;
}
template <typename Char>
Char* StrStr(Char *aSource, const Char *const aStr)
{
	return (Char*)((const Char*)aSource, aStr);
}

/**
 * Flips a piece of string in place (based on Kernighan & Ritchie's "Ansi C")
 *
 * @param  aBegin - the beginning of the overturned section
 * @param  aEnd   - end of the overturned section
 */
template <typename Char>
void StrReverse(Char *aBegin, Char *aEnd)
{
	if (!aBegin || !aEnd)
		return;

	for (; aBegin < aEnd; aBegin++, aEnd--)
	{
		Char chr = *aEnd;
		*aEnd = *aBegin;
		*aBegin = chr;
	}
}

} // namespace sld2

// Class for working with strings.
class CSldCompare
{
public:

	// Constructor
	CSldCompare(void);

	// Copy constructor
	CSldCompare(const CSldCompare& aRef);

	// Assignment operator
	CSldCompare& operator=(const CSldCompare& aRef);

	// Destructor
	~CSldCompare(void);

public:

	// Initialization
	ESldError Open(CSDCReadMy &aData, UInt32 aLanguageSymbolsTableCount, UInt32 aLanguageDelimiterSymbolsTableCount);


	// Returns the number of entries in the uppercase and lowercase character
	// mapping table for the current comparison table
	Int32 GetSymbolPairTableElementsCount(ESymbolPairTableTypeEnum aSymbolTable) const;

	// Returns an uppercase character code by record number and table type in
	// the uppercase and lowercase character mapping table for the current
	// comparison table
	UInt16 GetUpperSymbolFromSymbolPairTable(Int32 aIndex, ESymbolPairTableTypeEnum aSymbolTable) const;

	// Returns the lowercase character code by record number and table type in
	// the uppercase and lowercase character mapping table for the current
	// comparison table
	UInt16 GetLowerSymbolFromSymbolPairTable(Int32 aIndex, ESymbolPairTableTypeEnum aSymbolTable) const;


	// Converts a character to uppercase
	UInt16 ToUpperChr(UInt16 aChr) const;

	// Converts a character to lowercase
	UInt16 ToLowerChr(UInt16 aChr) const;

	// Converts a string to uppercase
	ESldError ToUpperStr(const UInt16* aStr, UInt16* aOutStr) const;

	// Converts a string to lowercase
	ESldError ToLowerStr(const UInt16* aStr, UInt16* aOutStr) const;

	// Returns a string of characters that have the same weight in all
	// collation tables as the passed character
	UInt16* GetSimilarMassSymbols(UInt16 aCh) const;

	// Returns a string of characters that have the same weight in the
	// collation table at the given index as the passed character
	UInt16* GetSimilarMassSymbols(UInt16 aCh, UInt32 aTableIndex) const;

	// Comparison of 2 strings according to the comparison table
	// (character weights are compared), indicating a specific table.
	Int32 StrICmp(const UInt16 *str1, const UInt16 *str2, UInt32 aTableIndex) const;

	// Comparison of 2 strings by comparison table (compare weights of characters)
	Int32 StrICmp(const UInt16 *str1, const UInt16 *str2) const;

	// Comparison of 2 strings according to the comparison table
	// (character weights are compared), indicating a specific table.
	Int32 StrICmp(SldU16StringRef str1, SldU16StringRef str2, UInt32 aTableIndex) const;

	// Comparison of 2 strings by comparison table (compare weights of characters)
	Int32 StrICmp(SldU16StringRef str1, SldU16StringRef str2) const;


	Int32 StrICmpByLanguage(const UInt16 *str1, const UInt16 *str2, ESldLanguage aLanguageCode) const;

	// Binary comparison of 2 strings
	static Int32 StrCmp(const UInt16 *str1, const UInt16 *str2);

	// Determines the length of a string
	static UInt32 StrLen(const UInt16 *aStr);

	// Copies a string
	static UInt32 StrCopy(UInt16* aStrDest, const UInt16* aStrSource);

	// Returns the number of characters in a string that have mass
	UInt32 StrEffectiveLen(const SldU16StringRef aStrSource, Int8 aEraseNotFoundSymbols = 1) const;

	// Copies a string ignoring zero mass characters
	UInt32 StrEffectiveCopy(UInt16* aStrOut, const UInt16* aStrSource, Int8 aEraseNotFoundSymbols = 1) const;

	// Copies a string ignoring zero mass characters
	void GetEffectiveString(const UInt16* aStrSource, SldU16String & aEffectiveString, Int8 aEraseNotFoundSymbols = 1) const;

	// Copies a string ignoring zero mass characters
	SldU16String GetEffectiveString(const SldU16StringRef aStrSource, Int8 aEraseNotFoundSymbols = 1) const;

	// Copies a line, ignoring delimiters at the beginning and end of the line
	SldU16String TrimDelimiters(const SldU16StringRef aStrSource) const;
	// Returns SldU16StringRef with no separators at the beginning and end of the string
	SldU16StringRef TrimDelimitersRef(const SldU16StringRef aStrSource) const;

	// Copies a string, ignoring zero characters at the beginning and end of the string
	SldU16String TrimIngnores(const SldU16StringRef aStrSource) const;
	// Returns SldU16StringRef without null characters at the beginning and end of the string
	SldU16StringRef TrimIngnoresRef(const SldU16StringRef aStrSource) const;

	// Swapping two elements in a sorted array
	static void Swap(UInt16 *aStr, Int32 aFirstIndex, Int32 aSecondIndex);

	// Quick sort
	static void DoQuickSort(UInt16 *aStr, Int32 aFirstIndex, Int32 aLastIndex);

	// Compare 2 strings case sensitive
	static Int32 StrCmpA(const UInt8 *str1, const UInt8 *str2);

	// Determines the length of a string
	static Int32 StrLenA(const UInt8 *str);

	// Copies a string
	static UInt32 StrCopyA(UInt8* aStrDest, const UInt8* aStrSource);

	// Copies a string of at most N length
	static UInt32 StrNCopyA(UInt8* aStrDest, const UInt8* aStrSource, UInt32 aSize);

	// Copies a string of at most N length
	static UInt32 StrNCopy(UInt16* aStrDest, const UInt16* aStrSource, UInt32 aSize);

	// Searches for the specified character in a string
	static UInt8* StrChrA(UInt8* aStr, UInt8 aChr);

	// Searches for the specified substring in a string
	static const UInt16* StrStr(const UInt16* aSourceStr, const UInt16* aSearchStr);

	// Searches for the specified substring in a string
	static const UInt8* StrStrA(const UInt8* aSourceStr, const UInt8* aSearchStr);

	// Converts a string from UTF-8 to UTF-16
	static UInt16 StrUTF8_2_UTF16(UInt16* unicode, const UInt8* utf8);

	// Converts a string from UTF-8 to UTF-32
	static UInt16 StrUTF8_2_UTF32(UInt32* unicode, const UInt8* utf8);

	// Converts a string from UTF16 to UTF8
	static UInt16 StrUTF16_2_UTF8(UInt8* utf8, const UInt16* unicode);

	// Converts a string from UTF16 to UTF32
	static UInt16 StrUTF16_2_UTF32(UInt32* unicode32, const UInt16* unicode16);

	// Converts a string from UTF32 to UTF8
	static UInt16 StrUTF32_2_UTF8(UInt8* utf8, const UInt32* unicode);

	// Converts a string from UTF32 to UTF16
	static UInt16 StrUTF32_2_UTF16(UInt16* unicode16, const UInt32* unicode32);

	// Flips a piece of string in place
	static ESldError StrReverse(UInt16* aBegin, UInt16* aEnd);

	// A function for converting from unicode to single-byte encoding, taking
	// into account the language in which the phrase is supposed to be.
	static ESldError Unicode2ASCIIByLanguage(const UInt16* aUnicode, UInt8* aAscii, ESldLanguage aLanguageCode);

	// The function of converting from single-byte encoding to unicode, taking
	// into account the language in which the phrase is intended.
	static ESldError ASCII2UnicodeByLanguage(const UInt8* aAscii, UInt16* aUnicode, ESldLanguage aLanguageCode);

	// Gets a number from its textual representation
	static ESldError StrToInt32(const UInt16* aStr, UInt32 aRadix, Int32* aNumber);

	// Gets a decimal floating point number from a string
	static ESldError StrToFloat32(const UInt16 *aStr, const UInt16 **aEnd, Float32 *aNumber);

	// Gets a number from its textual representation
	static ESldError StrToUInt32(const UInt16* aStr, UInt32 aRadix, UInt32* aNumber);

	// Gets a string from a number
	static ESldError UInt32ToStr(UInt32 aNumber, UInt16* aStr, UInt32 aRadix = 10);

	// Gets the number at which the string starts
	static ESldError StrToBeginInt32(const UInt16* aStr, UInt32 aRadix, Int32* aNumber);

	// Closing an object
	ESldError Close(void);

	// Returns the number of comparison tables
	ESldError GetTablesCount(UInt32* aCount) const;

	// Returns the language code of the comparison table by the table number
	ESldLanguage GetTableLanguage(UInt32 aTableIndex) const;

	// Returns a flag of whether the comparison table has a table of pairs of
	// upper and lower case characters of a certain type
	ESldError IsTableHasSymbolPairTable(UInt32 aTableIndex, ESymbolPairTableTypeEnum aTableType, UInt32* aFlag) const;

	// Checks if a character belongs to a specific language or to common
	// delimiters for all languages in the dictionary base
	// (aLang == SldLanguage :: Delimiters)
	ESldError IsSymbolBelongToLanguage(UInt16 aSymbolCode, ESldLanguage aLangCode, UInt32* aFlag, UInt32* aResultFlag) const;

	// Checks if a character is a delimiter in a specific language
	ESldError IsSymbolBelongToLanguageDelimiters(UInt16 aSymbolCode, ESldLanguage aLang, UInt32* aFlag, UInt32* aResultFlag) const;


	// Sets the comparison table for the specified language
	ESldError SetDefaultLanguage(ESldLanguage aLanguageCode);

	// Returns the default language code
	ESldLanguage GetDefaultLanguage() const;

	// Is there an additional comparison table for the current language
	bool IsAddTableDefine() const;

	// Returns the index of the additional comparison table for the current language
	UInt32 GetAddTableIndex() const { return m_DefaultAddTable; };

	// The method prepares a string for pattern matching
	UInt32 PrepareTextForAnagramSearch(UInt16* aDestStr, const UInt16* aSourceStr);
	// Encodes spec. characters in the query string ('&', '|', '(', ')', '!', '*', '?')
	static ESldError EncodeSearchQuery(UInt16* aDestStr, const UInt16* aSourceStr);
	// Encodes spec. characters in a word from a dictionary ('&', '|', '(', ')', '!', '*', '?')
	static ESldError EncodeSearchWord(UInt16* aDestStr, const UInt16* aSourceStr);

	// A method for comparing a pattern and a line for matching.
	UInt32 WildCompare(const UInt16* aWildCard, const UInt16* aText) const;

	UInt8 GetCompareLen(const UInt16* aWildCard, const UInt16* aText) const;
	// Anagram comparison method
	UInt8 AnagramCompare(UInt16* aSearchStr, const UInt16* aCurrentWord, UInt8* aFlagArray, UInt32 aSearchStrLen) const;

	// The method checks if the string contains delimiters
	UInt32 QueryIsExistDelim(const UInt16* aStr);

	// The method checks if the search term is "smart"
	static UInt32 IsSmartWildCardSearchQuery(const UInt16* aStr);
	// The method checks for the characters '*' and '?'
	static UInt32 IsWordHasWildCardSymbols(const UInt16* aStr);
	// The method adjusts the "smart" search query
	static ESldError CorrectSmartWildCardSearchQuery(const UInt16* aQuery, UInt16** aOut);
	// The method corrects the "stupid" search query
	static ESldError CorrectNonSmartWildCardSearchQuery(const UInt16* aQuery, UInt16** aOut);

	// The method checks if the search term is "smart"
	static UInt32 IsSmartFullTextSearchQuery(const UInt16* aStr);
	// The method adjusts the "smart" search query
	static ESldError CorrectSmartFullTextSearchQuery(const UInt16* aQuery, UInt16** aOut);
	// The method corrects the "stupid" search query
	static ESldError CorrectNonSmartFullTextSearchQuery(const UInt16* aQuery, UInt16** aOut);

	// The method checks for the characters '*' and '?' In the aStr string.
	UInt32 QueryIsExistWildSym(const UInt16* aStr);

	// The function calculates the edit distance
	Int32 FuzzyCompare(const UInt16 *aStr1, const UInt16 *aStr2, Int32 aStr1len, Int32 aStr2len, Int32 (*aFuzzyBuffer)[ARRAY_DIM]);

	// Converts a string of characters to a string of their masses
	ESldError GetStrOfMass(const UInt16* aSourceStr, SldU16String & aMassStr, Int8 aEraseZeroSymbols = 1, Int8 aUseMassForDigit = 0) const;
	// Converts a string of characters to a string of their masses, taking into account the delimiters
	ESldError GetStrOfMassWithDelimiters(const UInt16* aSourceStr, SldU16String & aMassStr, Int8 aEraseZeroSymbols = 1, Int8 aUseMassForDigit = 0) const;
	// Converts search pattern to mass pattern with special characters
	ESldError GetSearchPatternOfMass(const UInt16* aSourceStr, SldU16String & aMassStr, Int8 aUseMassForDigit = 0) const;

	// Splits the query into words.
	ESldError DivideQueryByParts(const UInt16 *aText, SldU16WordsArray& aTextWords) const;
	// Splits a query into words using passed delimiters
	ESldError DivideQueryByParts(const UInt16 *aText, const UInt16 *aDelimitersStr, SldU16WordsArray& aTextWords) const;

	// Splits the query into words.
	void DivideQueryByParts(SldU16StringRef aText, CSldVector<SldU16StringRef> &aTextWords) const;
	// Splits a query into words using passed delimiters
	void DivideQueryByParts(SldU16StringRef aText, const UInt16 *aDelimitersStr, CSldVector<SldU16StringRef> &aTextWords) const;

	// Splits a query into words, returning alternatives for partial delimiters
	ESldError DivideQuery(const UInt16 *aText, SldU16WordsArray& aTextWords, SldU16WordsArray& aAltrnativeWords) const;

	// The function returns the type of alphabet for the entered text in the context of the current language
	UInt32 GetAlphabetTypeByText(const UInt16 *text) const;

	// Checks if the given character is ignored in the given collation table
	Int8 IsZeroSymbol(UInt16 aChar, UInt32 aTableIndex) const;
	// Checks if the given character is ignored in the current default sort table
	Int8 IsZeroSymbol(UInt16 aChar) const;

	// Checks if the given character is a delimiter in the given sort table
	Int8 IsDelimiter(UInt16 aChar, UInt32 aTableIndex) const;
	// Checks if the given character is a delimiter in the current default sort table
	Int8 IsDelimiter(UInt16 aChar) const;

	// Checks if the given character is a partial delimiter in the given sort table
	Int8 IsHalfDelimiter(UInt16 aChar, UInt32 aTableIndex) const;
	// Checks if the given character is a partial delimiter in the current default sort table
	Int8 IsHalfDelimiter(UInt16 aChar) const;

	// Checks if the given character is an Emoji character
	static Int8 IsEmoji(const UInt16 aChar, const EEmojiTypes aType = eSlovoedEmoji);

	// Checks if the given character is whitespace
	static Int8 IsWhitespace(const UInt16 aChar);

	// Checks if the given character is significant in at least one of the tables
	Int8 IsMarginalSymbol(const UInt16 aChar) const;

	// Checks the integrity of parentheses
	Int8 CheckBracket(const UInt16 *aText) const;

	UInt32 GetTableVersion() const { return m_CMPTable[0].Header->Version; }

	// "Returns" the delimiter string in the sort table for the specified language
	ESldError GetDelimiters(ESldLanguage aLangCode, const UInt16 **aDelimitersStr, UInt32 *aDelimitersCount) const;
	// "Returns" the delimiter string in the current default sort table
	ESldError GetDelimiters(const UInt16 **aDelimitersStr, UInt32 *aDelimitersCount) const;

	// Adds the selected selector after all emoji characters in the string
	static ESldError AddEmojiSelector(SldU16String & aString, const EEmojiTypes aType = eSlovoedEmoji, const UInt16 aSelector = SLD_DEFAULT_EMOJI_SELECTOR);

	// Removes selectors after all emoji characters in a string
	static ESldError ClearEmojiSelector(SldU16String & aString, const EEmojiTypes aType = eSlovoedEmoji);

	// Converts a 4-character string to a 4-byte number (e.g. language code, base ID)
	static UInt32 UInt16StrToUInt32Code(const SldU16StringRef aString);

	// Returns the next largest simple character
	UInt16 GetNextMassSymbol(const UInt16 aSymbol) const;

	// Returns a word by position in a phrase
	SldU16StringRef GetWordByPosition(const SldU16StringRef aPhrase, const UInt32 aPos) const;

	// Replaces the selected word in a phrase
	void ReplaceWordInPhraseByIndex(SldU16String& aPhrase, const SldU16StringRef aNewWord, const UInt32 aWordIndex) const;

  SldU16WordsArray ExpandBrackets(SldU16StringRef aText) const;

private:

	// Object cleaning
	void Clear(void);

	// Returns the mass of a character
	UInt16 GetMass(UInt16 aChr, const UInt16 *aSimpleTable, UInt16 aNotFound) const;

	// Returns the mass for a complex symbol
	UInt32 GetComplex(const UInt16 *str, UInt16 index, UInt16 *mass, const CMPComplexType *complex) const;

	// Method of comparing pattern and word for matching
	Int8 DoWildCompare(const UInt16* aTemplate, const UInt16* aText) const;

	// Gets a number from its textual representation, main function
	static ESldError StrToInt32Base(const UInt16* aStr, UInt32 aRadix, Int32* aNumber);

private:

	// Comparison tables
	sld2::DynArray<TCompareTableSplit> m_CMPTable;

	// Comparison table information
	sld2::DynArray<TCMPTableElement> m_CMPTableInfo;

	// Default comparison table.
	UInt32				m_DefaultTable;

	// Optional default comparison table (if any).
	UInt32				m_DefaultAddTable;

	// Array of pointers to language character tables (tables for each
	// language + common table of separator characters for all languages)
	sld2::DynArray<CSldSymbolsTable>	m_LanguageSymbolsTable;

	// Array of pointers to language-specific delimiter tables
	sld2::DynArray<CSldSymbolsTable>	m_LanguageDelimiterSymbolsTable;
};

#endif //_SLD_COMPARE_H_
