#ifndef _C_SLD_DICTIONARY_
#define _C_SLD_DICTIONARY_

#include "SldVersionInfo.h"
#include "SldError.h"
#include "SldPlatform.h"
#include "SldTypes.h"
#include "SldCompare.h"
#include "ISldList.h"
#include "SldList.h"
#include "SldSearchList.h"
#include "SldCustomList.h"
#include "SldMergedList.h"
#include "SldArticles.h"
#include "ISldLayerAccess.h"
#include "SldListInfo.h"
#include "SldStyleInfo.h"
#include "SldLocalizedString.h"
#include "SldImageManager.h"
#include "SldSceneManager.h"
#include "SldVideoItem.h"
#include "SldVideoElement.h"
#include "SldMetadataParser.h"
#include "SldSerialNumber.h"
#include "SldSpeexDecoder.h"
#include "SldWavDecoder.h"
#include "SldMp3Decoder.h"
#include "SldOggDecoder.h"
#include "SldSpeexSinTable.h"
#include "SldMorphology.h"

// Class advance declaration
class CSldCSSDataManager;
class CSldMetadataManager;

// Dictionary class.
class CSldDictionary
{
	// We declare it friendly, because CSldMergedDictionary will do what others will not be allowed to do.
	friend class CSldMergedDictionary;
	friend ESldError GetMorphoForms(const UInt16 *aText, CSldDictionary *aDict, UInt32 aLangFrom, CSldVector<TSldMorphologyWordStruct>& aMorphologyForms, UInt32 aWithoutRequest, const EMorphoFormsType aType);

public:

	// Standard constructor
	CSldDictionary(void);

	// Destructor
	virtual ~CSldDictionary(void);

public:

	// Opens a dictionary
	virtual ESldError Open(ISDCFile *aFile, ISldLayerAccess *aLayerAccess);

	// Closes the dictionary
	virtual ESldError Close(void);

	// Gets the morphology for the given language code and optionally the morphology base id
	ESldError GetMorphology(UInt32 aLanguageCode, MorphoData **aMorphoData, UInt32 aDictId = 0);

	// Sets the wordlist as current.
	virtual ESldError SetCurrentWordlist(Int32 aIndex);

	// Gets the index of the current wordlist
	virtual ESldError GetCurrentWordList(Int32* aIndex) const;

	// Sets the base position for the current wordlist, relative to which word numbers will be counted.
	// MAX_UINT_VALUE means to set the base position to the beginning of the word list.
	// If a word at the current nesting level with local number aIndex has hierarchy sublevels,
	// then calling SetBase (aIndex) means going inside this directory (going one level down)
	ESldError SetBase(Int32 aIndex);
	// Set the base position for the word list by the list number, relative to which the word numbers will be counted.
	// MAX_UINT_VALUE means to set the base position to the beginning of the word list.
	// If a word at the current nesting level with local number aIndex has hierarchy sublevels,
	// then calling SetBase (aIndex) means going inside this directory (going one level down)
	ESldError SetBase(Int32 aListIndex, Int32 aIndex);

	// Gets a word by the local word number for the current wordlist (within the current nesting level)
	ESldError GetWordByIndex(Int32 aWordIndex);
	// Gets a word by local word number and wordlist number (within the current nesting level)
	ESldError GetWordByIndex(Int32 aListIndex, Int32 aWordIndex);

	// Gets the total number of words in a word list by the list number
	ESldError GetTotalWordCount(Int32 aListIndex, Int32* aCount);

	// Gets a word by the global word number in the current wordlist
	ESldError GetWordByGlobalIndex(Int32 aGlobalWordIndex);
	// Gets a word by wordlist number, global word number in wordlist, and spelling number
	ESldError GetWordByGlobalIndex(Int32 aListIndex, Int32 aGlobalWordIndex, Int32 aVariantIndex, UInt16** aWord);

	// Searches for the closest word in the current wordlist that is greater than or equal to the given word
	ESldError GetWordByText(const UInt16* aText);
	// Searches for the closest word in the specified word list that is greater than or equal to the given
	ESldError GetWordByText(Int32 aListIndex, const UInt16* aText);

	// Searches for the closest word in the current wordlist that is greater than or equal to the given word
	// Works in unsorted lists, for sorted by sorting lists it searches by Show-variant (starting from version 112+ databases)
	ESldError GetWordByTextExtended(const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag = 0);
	// Searches for the closest word in the specified word list that is greater than or equal to the given
	// Works in unsorted lists, for sorted by sorting lists it searches by Show-variant (starting from version 112+ databases)
	ESldError GetWordByTextExtended(Int32 aListIndex, const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag = 0);

	// Searches for words that match the mass specified in the current word list
	// Works in unsorted lists, for sorted by sorting lists it searches by Show-variant (starting from version 112+ databases)
	ESldError GetWordSetByTextExtended(const UInt16* aText, CSldVector<Int32>& aWordSet);
	// Searches for words that match the mass given in the specified word list
	// Works in unsorted lists, for sorted by sorting lists it searches by Show-variant (starting from version 112+ databases)
	ESldError GetWordSetByTextExtended(Int32 aListIndex, const UInt16* aText, CSldVector<Int32>& aWordSet);

	// Searches for a word in the current word list in all possible ways
	ESldError FindWordBySpecificRule(const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag = 1);
	// Searches for a word in the specified word list in all possible ways
	ESldError FindWordBySpecificRule(Int32 aListIndex, const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag = 1);

	// Searches for a word in the current list of words that matches all spellings passed, in all possible ways
	ESldError FindWordBySpecificRule(TWordVariantsSet* aWordVariantsSet, UInt32* aResultFlag, UInt32 aWordSearchType = eShowVariantFuzzyMatch, UInt32 aActionsOnFailFlag = 1);
	// Searches for a word in the specified list of words that matches all spelling options provided, in all possible ways
	ESldError FindWordBySpecificRule(Int32 aListIndex, TWordVariantsSet* aWordVariantsSet, UInt32* aResultFlag, UInt32 aWordSearchType = eShowVariantFuzzyMatch, UInt32 aActionsOnFailFlag = 1);

	// Searches for the most similar word in the current wordlist that is equal to the given
	ESldError GetMostSimilarWordByText(const UInt16* aText, UInt32* aResultFlag);
	// Searches for the most similar word in the specified word list that is equal to the given
	ESldError GetMostSimilarWordByText(Int32 aListIndex, const UInt16* aText, UInt32* aResultFlag);

	// Gets information about the last found word in the current wordlist
	ESldError GetCurrentWord(Int32 aVariantIndex, UInt16 **aWord);
	// Gets information about the last found word in the specified wordlist
	ESldError GetCurrentWord(Int32 aListIndex, Int32 aVariantIndex, UInt16 **aWord);

	// Gets information about the label to roll
	ESldError GetCurrentWordLabel(UInt16** aLabel);

	// Gets the local number of the current word for the current wordlist (within the current nesting level)
	ESldError GetCurrentIndex(Int32* aIndex);
	// Gets the local number of the current word for the specified wordlist (within the current nesting level)
	ESldError GetCurrentIndex(Int32 aListIndex, Int32* aIndex);

	// Gets the global number of the current word in the current wordlist (word number within the entire wordlist)
	ESldError GetCurrentGlobalIndex(Int32* aIndex);
	// Gets the global number of the current word in the specified wordlist (word number within the entire wordlist)
	ESldError GetCurrentGlobalIndex(Int32 aListIndex, Int32* aIndex);

	// Converts the local word number within the current nesting level in the current wordlist
	// to a global word number within the current wordlist
	ESldError LocalIndex2GlobalIndex(Int32 aLocalIndex, Int32* aGlobalIndex);
	// Converts the local word number within the current nesting level in the specified wordlist
	// to the global word number within the specified wordlist
	ESldError LocalIndex2GlobalIndex(Int32 aListIndex, Int32 aLocalIndex, Int32* aGlobalIndex);

	// Gets the number of a wordlist by the local word number in the current wordlist.
	// For a regular list (not search), SLD_DEFAULT_LIST_INDEX is returned.
	// For the search list, the number of the word list in which the word was found is returned.
	// For a full-text search list, the index of the underlying list is returned for the first search result.
	ESldError GetRealListIndex(Int32 aLocalWordIndex, Int32* aRealListIndex);
	// Gets the number of a wordlist by the local word number in the specified wordlist.
	// For a regular list (not search), SLD_DEFAULT_LIST_INDEX is returned.
	// For the search list, the number of the word list in which the word was found is returned.
	// For a full-text search list, the index of the underlying list is returned for the first search result.
	ESldError GetRealListIndex(Int32 aListIndex, Int32 aLocalWordIndex, Int32* aRealListIndex);

	// Gets the global word number based on the local word number in the current wordlist.
	// For a regular list (not search), SLD_DEFAULT_WORD_INDEX is returned.
	// For the search list, the global word number is returned from the word list in which the word was found.
	// For a full-text search list, the global number of the first result from the underlying list is returned.
	ESldError GetRealGlobalIndex(Int32 aLocalWordIndex, Int32* aGlobalWordIndex);
	// Gets the global word number based on the local word number in the specified wordlist.
	// For a regular list (not search), SLD_DEFAULT_WORD_INDEX is returned.
	// For the search list, the global word number is returned from the word list in which the word was found.
	// For a full-text search list, the global number of the first result from the underlying list is returned.
	ESldError GetRealGlobalIndex(Int32 aListIndex, Int32 aLocalWordIndex, Int32* aGlobalWordIndex);

	// Gets the number of real indices in the current wordlist.
	ESldError GetRealIndexesCount(Int32 aLocalWordIndex, Int32* aRealIndexesCount);
	// Gets the number of real indices in the specified wordlist
	ESldError GetRealIndexesCount(Int32 aListIndex, Int32 aLocalWordIndex, Int32* aRealIndexesCount);

	// Gets the real indexes of the list and the entries in the real list in by the local word number in the current wordlist
	ESldError GetRealIndexes(Int32 aLocalWordIndex, Int32 aTrnslationIndex, Int32* aRealListIndex, Int32* aGlobalWordIndex);
	// Retrieves the real indices of the list and the entries in the real list in by the local word number in the specified word list
	virtual ESldError GetRealIndexes(Int32 aListIndex, Int32 aLocalWordIndex, Int32 aTrnslationIndex, Int32* aRealListIndex, Int32* aGlobalWordIndex);

	// Gets the picture number of the current word in the current wordlist
	ESldError GetCurrentWordPictureIndex(CSldVector<Int32> & aPictureIndexes);
	// Gets the picture number of the current word in the specified wordlist
	ESldError GetCurrentWordPictureIndex(Int32 aListIndex, CSldVector<Int32> & aPictureIndexes);

	// Gets the video number of the current word in the current wordlist
	ESldError GetCurrentWordVideoIndex(Int32* aVideoIndex);
	// Gets the video number of the current word in the specified wordlist
	ESldError GetCurrentWordVideoIndex(Int32 aListIndex, Int32* aVideoIndex);

	// Gets the scene number of the current word in the current wordlist
	ESldError GetCurrentWordSceneIndex(Int32* aVideoIndex);
	// Gets the scene number of the current word in the specified wordlist
	ESldError GetCurrentWordSceneIndex(Int32 aListIndex, Int32* aVideoIndex);

	// Gets a vector of voice indices of the current word in the current wordlist
	ESldError GetCurrentWordSoundIndex(CSldVector<Int32> & aSoundIndexes);
	// Gets a vector of voice indices of the current word in the specified wordlist
	ESldError GetCurrentWordSoundIndex(Int32 aListIndex, CSldVector<Int32> & aSoundIndexes);

	// Gets a picture by number
	virtual ESldError GetWordPicture(Int32 aPictureIndex, Int32 aMaxSideSize, TImageElement* aImageElement);

	// Gets information about the uploaded image
	ESldError GetPictureInfo(TImageElement* aImageElement, UInt32* aFormatType, TSizeValue* aWidth = 0, TSizeValue* aHeight = 0) const;

	// Frees up resources with a picture
	ESldError ReleaseWordPicture(TImageElement* aImageElement) const;

	// Receives a video by its representative with content validation
	ESldError GetWordVideo(const UInt32 aVideoId, CSldVideoElement* aVideoElement);

	// Frees up resources with video
	ESldError ReleaseWordVideo(CSldVideoElement* aVideoElement);

	// Gets an abstract resource by number
	ESldError GetWordAbstractResource(Int32 aAbstractResourceIndex, TElement* aAbstractResourceElement);

	// Gets a binary resource by number
	ESldError GetBinaryResourceByIndex(const Int32 aResourceIndex, TElement* aResource);

	// Gets a 3D scene by number
	ESldError GetWordScene(Int32 aSceneIndex, TElement* aSceneElement);

	// Gets the model by number
	ESldError GetWordMesh(Int32 aMeshIndex, TElement* aMeshElement);

	// Retrieves material by number
	ESldError GetWordMaterial(Int32 aMaterialIndex, TElement* aModelElement);

	// Frees up 3d scene resources
	ESldError ReleaseWordResources(TElement* aSceneElement);

	// Gets voiceovers ready for playing by voiceovers index
	ESldError PlaySoundByIndex(Int32 aSoundIndex, UInt8 aIsLast, UInt32* aStartPos, UInt32 aExternFlag = 0);

	// Gets the voice acting ready for playing by the voice acting index indicating where to play from.
	ESldError PlaySoundByIndexWithPosition(Int32 aSoundIndex, UInt8 aIsLast, UInt32* aStartPos,UInt8 aBeginPlay,UInt32 aExternFlag = 0);

	// Receives voiceovers, ready for playing, based on the corresponding text from the current sheet
	ESldError PlaySoundByText(const UInt16 *aText, UInt32 *aResultFlag);

	// Returns information about the voice acting (?)
	ESldError GetSoundInfo(Int32 aSoundIndex, TSoundElement* aSoundInfo);

	// Generates a tone that is used in case of unregistered voice acting (?)
	static ESldError GenerateToneSound(UInt8** aDataPtr, UInt32* aDataSize, UInt32* aFrequency);

	// Returns the number of words of the current wordlist at the current nesting level.
	ESldError GetNumberOfWords(Int32 *aNumberOfWords);
	// Returns the number of words for the specified wordlist at the current nesting level.
	ESldError GetNumberOfWords(Int32 aListIndex, Int32 *aNumberOfWords);

	// Returns the number of wordlists.
	virtual ESldError GetNumberOfLists(Int32 *aNumberOfLists) const;

	// Возвращает флаг, имеет или нет слово с локальным номером в текущем списке слов поддерево иерархии
	ESldError isWordHasHierarchy(Int32 aLocalWordIndex, UInt32* aIsHierarchy, EHierarchyLevelType* aLevelType = NULL);
	// Возвращает флаг, имеет или нет слово с локальным номером в указанном списке слов поддерево иерархии
	ESldError isWordHasHierarchy(Int32 aListIndex, Int32 aLocalWordIndex, UInt32* aIsHierarchy, EHierarchyLevelType* aLevelType = NULL);

	// Возвращает количество переводов слова по локальному номеру слова в текущем списке слов
	ESldError GetNumberOfTranslations(Int32 aIndex, Int32* aNumberOfTraslations);
	// Возвращает количество переводов слова по локальному номеру слова в указанном списке слов
	ESldError GetNumberOfTranslations(Int32 aListIndex, Int32 aIndex, Int32* aNumberOfTraslations);
	
	// Производит перевод указанной статьи по локальному номеру слова в текущем списке слов и номеру перевода
	ESldError Translate(Int32 aIndex, Int32 aVariantIndex, ESldTranslationFullnesType aFullness, UInt32 aStartBlock = 0, UInt32 aEndBlock = -1);
	// Производит перевод указанной статьи по локальному номеру слова в указанном списке слов и номеру перевода
	ESldError Translate(Int32 aListIndex, Int32 aIndex, Int32 aVariantIndex, ESldTranslationFullnesType aFullness, UInt32 aStartBlock = 0, UInt32 aEndBlock = -1);

	// Производит перевод указанной статьи по глобальному номеру статьи
	virtual ESldError TranslateByArticleIndex(Int32 aIndex, ESldTranslationFullnesType aFullness, UInt32 aStartBlock = 0, UInt32 aEndBlock = -1);

	// Производит получение остатков перевода 
	virtual ESldError TranslateContinue(ESldTranslationFullnesType aFullness);
	// Производит завершение перевода и подготовку к получению нового перевода
	virtual ESldError TranslateClear();

	// Возвращает путь к текущему положению в каталоге в текущем списке слов
	ESldError GetCurrentPath(TCatalogPath *aPath);
	// Возвращает путь к текущему положению в каталоге в указанном списке слов
	ESldError GetCurrentPath(Int32 aListIndex, TCatalogPath *aPath);

	// Переходит по указанному пути
	ESldError GoToByPath(TCatalogPath* aPath, ESldNavigationTypeEnum aNavigationType);
	
	// Поднимаемся в каталоге в текущем списке слов на уровень выше текущего
	ESldError GoToLevelUp(void);
	// Поднимаемся в каталоге в указанном списке слов на уровень выше текущего
	ESldError GoToLevelUp(Int32 aListIndex);
	
	// Возвращает название родительской категории для текущего слова в текущем списке слов
	ESldError GetCurrentParentWordName(UInt16** aText);
	// Возвращает название родительской категории для текущего слова в указанном списке слов
	ESldError GetCurrentParentWordName(Int32 aListIndex, UInt16** aText);
	
	// Возвращает список названий родительских категорий, разделенных строкой-разделителем, по глобальному номеру слова в текущем списке слов
	ESldError GetParentWordNamesByGlobalIndex(Int32 aGlobalWordIndex, UInt16** aText, const UInt16* aSeparatorText, Int32 aMaxLevelCount = -1);
	// Возвращает список названий родительских категорий, разделенных строкой-разделителем, по глобальному номеру слова в указанном списке слов
	ESldError GetParentWordNamesByGlobalIndex(Int32 aListIndex, Int32 aGlobalWordIndex, UInt16** aText, const UInt16* aSeparatorText, Int32 aMaxLevelCount = -1);
	
	// Возвращает глобальный номер слова-родителя по глобальному номеру слова в текущем списке слов
	ESldError GetParentWordGlobalIndex(Int32 aGlobalWordIndex, Int32* aParentWordGlobalIndex);
	// Возвращает глобальный номер слова-родителя по глобальному номеру слова в указанном списке слов
	ESldError GetParentWordGlobalIndex(Int32 aListIndex, Int32 aGlobalWordIndex, Int32* aParentWordGlobalIndex);
	
	// Возвращает границы глобальных номеров слов-потомков по глобальному номеру слова-родителя в текущем списке слов
	// Получается диапазон глобальных индексов вида [aFirstGlobalIndex, aLastGlobalIndex) - левая граница включается, правая - нет
	// Если у слова-родителя нет потомков, оба глобальных индекса будут равны -1
	ESldError GetChildrenWordsGlobalIndexes(Int32 aParentGlobalWordIndex, Int32* aFirstGlobalIndex, Int32* aLastGlobalIndex);
	// Возвращает границы глобальных номеров слов-потомков по глобальному номеру слова-родителя в указанном списке слов
	// Получается диапазон глобальных индексов вида [aFirstGlobalIndex, aLastGlobalIndex) - левая граница включается, правая - нет
	// Если у слова-родителя нет потомков, оба глобальных индекса будут равны -1
	ESldError GetChildrenWordsGlobalIndexes(Int32 aListIndex, Int32 aParentGlobalWordIndex, Int32* aFirstGlobalIndex, Int32* aLastGlobalIndex);

	// Возвращает путь к слову по глобальному номеру слова (без учета иерархии) в текущем списке слов (любом -  в каталоге, словаре и т.д.)
	// Последний элемент в списке элементов пути содержит локальный номер слова на конечном уровне вложенности
	ESldError GetPathByGlobalIndex(Int32 aGlobalWordIndex, TCatalogPath* aPath);
	// Возвращает путь к слову по глобальному номеру слова (без учета иерархии) в указанном списке слов (любом -  в каталоге, словаре и т.д.)
	// Последний элемент в списке элементов пути содержит локальный номер слова на конечном уровне вложенности
	ESldError GetPathByGlobalIndex(Int32 aListIndex, Int32 aGlobalWordIndex, TCatalogPath* aPath);
	

	// Возвращает класс, хранящий информацию о свойствах текущего списка слов
	virtual ESldError GetWordListInfo(const CSldListInfo **aListInfo) const;
	// Возвращает класс, хранящий информацию о свойствах списка слов
	virtual ESldError GetWordListInfo(Int32 aListIndex, const CSldListInfo **aListInfo) const;
	
	// Возвращает флаг того, сортированный или нет список слов с указанным индексом
	ESldError IsListSorted(Int32 aListIndex, UInt32* aIsSorted) const;
	
	// Возвращает флаг того, сопоставлены или нет некоторым словам из списка слов картинки
	ESldError IsListHasPicture(Int32 aListIndex, UInt32* aIsPicture) const;
	
	// Возвращает флаг того, сопоставлена или нет некоторым словам из списка слов озвучка
	ESldError IsListHasSound(Int32 aListIndex, UInt32* aIsSound) const;

	// Возвращает флаг того, сопоставлено или нет некоторым словам из списка слов видео
	ESldError IsListHasVideo(Int32 aListIndex, UInt32* aIsVideo);

	// Возвращает флаг того, сопоставлено или нет некоторым словам из списка слов 3d сцены
	ESldError IsListHasScene(Int32 aListIndex, UInt32* aIsScene);

	// Возвращает флаг того, имеет или нет список слов локализованные строки
	ESldError IsListHasLocalizedStrings(Int32 aListIndex, UInt32* aIsLocalizedStrings) const;
	
	
	// Возвращает номер старшей версии словарной базы
	virtual ESldError GetDictionaryMajorVersion(UInt32* aVersion, const Int32 aDictionaryIndex = 0) const;
	
	// Возвращает номер младшей версии словарной базы
	virtual ESldError GetDictionaryMinorVersion(UInt32* aVersion, const Int32 aDictionaryIndex = 0) const;

	// Возвращает структуру с дополнительной информацией по базе
	ESldError GetAdditionalInfo(TAdditionalInfo *aAdditionalInfo);

	// Возвращает строку с аннотацией к базе
	ESldError GetAnnotation(UInt16 **aAnnotation);
	
	// Возвращает константу, которая обозначает бренд словарной базы (см. EDictionaryBrandName)
	virtual ESldError GetDictionaryBrand(UInt32* aBrand, const Int32 aDictionaryIndex = 0) const;
	
	// Возвращает хэш словарной базы
	virtual ESldError GetDictionaryHash(UInt32* aHash, const Int32 aDictionaryIndex = 0) const;

	// Возвращает ID словарной базы
	virtual ESldError GetDictionaryID(UInt32* aDictID, const Int32 aDictionaryIndex = 0) const;

	// Возвращает ID словарной базы, сконвертированный в строку
	ESldError GetDictionaryStringID(UInt16* aDictID) const;
	
	// Возвращает маркетинговое общее количество слов в словаре
	ESldError GetMarketingTotalWordsCount(Int32* aWordsCount) const;

	// Возвращает общее количество картинок в словаре
	virtual ESldError GetTotalPictureCount(Int32* aCount) const;

	// Возвращает общее количество озвучек в словаре
	virtual ESldError GetTotalSoundCount(Int32* aCount) const;

	// Возвращает размер максимального слова в данном словаре, в символах
	ESldError GetTranslateMaxBlockSize(Int32* aWordMaxSize) const;

	// Возвращает размер максимального слова в данном словаре, в символах
	ESldError GetHeadwordMaxSize(Int32* aWordMaxSize) const;
	
	// Получает основной язык словаря
	ESldLanguage GetLanguageFrom() const;
	
	// Получает основной язык текущего списка слов
	ESldLanguage GetCurrentLanguageFrom() const;
	// Получает основной язык указанного списка слов
	ESldLanguage GetListLanguageFrom(Int32 aListIndex) const;
	
	// Получает язык перевода словаря
	ESldLanguage GetLanguageTo() const;
	
	// Получает язык перевода текущего списка слов
	ESldLanguage GetCurrentLanguageTo() const;
	// Получает язык перевода указанного списка слов
	ESldLanguage GetListLanguageTo(Int32 aListIndex) const;
	
	// Получает количество кодов использования текущего списка слов
	ESldError GetCurrentListUsageCount(Int32* aCount);
	// Получает количество кодов использования указанного списка слов
	ESldError GetListUsageCount(Int32 aListIndex, Int32* aCount);
	// Получает количество кодов использования указанного списка слов
	ESldError GetUsageCountByListIndex(Int32 aListIndex, Int32* aCount);
	
	// Получает код использования текущего списка слов по индексу кода
	ESldError GetCurrentListUsage(Int32 aIndex, UInt32* aUsage);
	// Получает код использования указанного списка слов по индексу кода
	ESldError GetListUsage(Int32 aListIndex, Int32 aUsageIndex, UInt32* aUsage);
	// Получает код использования указанного списка слов по индексу кода
	ESldError GetUsageByListIndex(Int32 aListIndex, Int32 aUsageIndex, UInt32* aUsage);

	// Возвращает количество стилей
	virtual UInt32 GetNumberOfStyles() const;
	// Возвращает класс, хранящий информацию об указанном стиле
	virtual ESldError GetStyleInfo(Int32 aIndex, const CSldStyleInfo **aStyleInfo) const;
	// Устанавливает для стиля вариант отображения по умолчанию
	ESldError SetDefaultStyleVariant(Int32 aStyleIndex, Int32 aVariantIndex);
	// Устанавливает для всех стилей вариант отображения по умолчанию
	ESldError SetDefaultStyleVariant(Int32 aVariantIndex);

	// Производит поиск слов по шаблону в текущем списке слов. Начиная с текущего уровня (и во всех вложенных уровнях).
	ESldError DoWildCardSearch(TExpressionBox* aExpressionBox, Int32 aMaximumWords);
	// Производит поиск слов по шаблону в указанном списке слов. Начиная с текущего уровня (и во всех вложенных уровнях).
	ESldError DoWildCardSearch(Int32 aListIndex, TExpressionBox* aExpressionBox, Int32 aMaximumWords);

	// Производит поиск похожих слов в текущем списке слов. Начиная с текущего уровня (и во всех вложенных уровнях).
	ESldError DoFuzzySearch(const UInt16 *aText, Int32 aMaximumWords, Int32 aMaximumDifference, EFuzzySearchMode aSearchMode = eFuzzy_CompareSortTable);
	// Производит поиск похожих слов в указанном списке слов. Начиная с текущего уровня (и во всех вложенных уровнях).
	ESldError DoFuzzySearch(Int32 aListIndex, const UInt16 *aText, Int32 aMaximumWords, Int32 aMaximumDifference, EFuzzySearchMode aSearchMode = eFuzzy_CompareSortTable);
	
	// Производит поиск анаграмм в текущем списке слов. Начиная с текущего уровня (и во всех вложенных уровнях).
	ESldError DoAnagramSearch(const UInt16 *aText, Int32 aMaximumWords = MAX_NUMBER_OF_WORDS_IS_UNDEFINED);
	// Производит поиск анаграмм в указанном списке слов. Начиная с текущего уровня (и во всех вложенных уровнях).
	ESldError DoAnagramSearch(Int32 aListIndex, const UInt16 *aText, Int32 aMaximumWords = MAX_NUMBER_OF_WORDS_IS_UNDEFINED);
	
	// Производит поиск слов в текущем списке слов при вводе с учетом возможных опечаток. Начиная с текущего уровня (и во всех вложенных уровнях).
	ESldError DoSpellingSearch(const UInt16 *aText, Int32 aMaximumWords = MAX_NUMBER_OF_WORDS_IS_UNDEFINED);
	// Производит поиск слов в указанном списке слов при вводе с учетом возможных опечаток. Начиная с текущего уровня (и во всех вложенных уровнях).
	ESldError DoSpellingSearch(Int32 aListIndex, const UInt16 *aText, Int32 aMaximumWords = MAX_NUMBER_OF_WORDS_IS_UNDEFINED);
	
	// Производит полнотекстовый поиск в текущем списке слов
	ESldError DoFullTextSearch(const UInt16* aText, Int32 aMaximumWords);
	// Производит полнотекстовый поиск в указанном списке слов
	virtual ESldError DoFullTextSearch(Int32 aListIndex, const UInt16* aText, Int32 aMaximumWords);
	
	// Производит сортировку поискового списка слов по релевантности шаблону поиска
	ESldError SortSearchListRelevant(Int32 aListIndex, const UInt16* aText, const CSldVector<TSldMorphologyWordStruct>& aMorphologyForms, ESldFTSSortingTypeEnum aMode);

	// Производит сортировку поискового списка слов по заданному варианту написания
	ESldError SortResultByVariantType(const Int32 aListIndex, const EListVariantTypeEnum aType);

	// Удаляет все поисковые списки (очистка результатов поиска)
	ESldError ClearSearch(void);

	// Удаляет определенный пользовательский список
	ESldError RemoveCustomList(Int32 aListIndex = -1);


	// Возвращает класс, хранящий информацию о строчках для всего словаря
	virtual ESldError GetLocalizedStrings(const CSldLocalizedString** aStrings, const Int32 aDictionaryIndex = 0) const;
	
	// Возвращает класс, хранящий информацию о строчках для конкретного списка слов
	ESldError GetLocalizedStringsByListIndex(Int32 aListIndex, const CSldListLocalizedString** aStrings) const;
	
	// Возвращает класс, занимающийся сравнением строк
	virtual ESldError GetCompare(const CSldCompare** aCMP) const;

	// Регистрируем данный словарь с помощью серийного номера
	ESldError RegisterDictionary(const UInt16 *aSerialNumber);
	
	// Возвращает флаг того, зарегистрирован словарь или нет
	ESldError IsDictionaryRegistered(UInt32* aIsRegistered) const;
	
	// Возвращает флаг того, есть ли в словаре флаг фичи
	bool HasFeature(ESldFeatures aFeature) const;

	// Возвращает флаг того, есть ли в словаре SkeletalMode
	bool IsDictionaryHasSkeletalMode() const;
	
	// Возвращает флаг того, есть ли в словаре Taboo
	bool IsDictionaryHasTaboo() const;
	
	// Возвращает флаг того, есть ли в словаре Stress
	bool IsDictionaryHasStress() const;

    /// Возвращает флаг того, есть ли в словаре темная тема
    bool IsDictionaryHasDarkTheme() const;

	// Возвращает флаг того, есть ли в словаре блоки switch с тематиками
	bool IsDictionaryHasSwitchThematics() const;
	

	// Возвращает флаг того, есть ли в словаре в таблицах сравнения таблицы пар соответствий символов верхнего и нижнего регистров определенного типа
	ESldError IsDictionaryHasCmpSymbolPairTable(UInt32* aFlag, ESymbolPairTableTypeEnum aTableType = eSymbolPairTableType_Dictionary) const;	
	
	// Проверяет, принадлежит ли символ определеленному языку,
	// либо к общим символам-разделителям для всех языков словарной базы (aLang == SldLanguage::Delimiters)
	ESldError IsSymbolBelongToLanguage(UInt16 aSymbolCode, ESldLanguage aLang, UInt32* aFlag, UInt32* aResultFlag);
	
	// Проверяет, является ли символ разделителем в конкретном языке
	ESldError IsSymbolBelongToLanguageDelimiters(UInt16 aSymbolCode, ESldLanguage aLang, UInt32* aFlag, UInt32* aResultFlag);
	
	// Определение языка по строке текста
	ESldError RecognizeLanguage(const UInt16* aText, ESldLanguage& aLang, UInt32& aResultFlag);
	
	
	// Переключение текущего направления словаря на обратное
	ESldError SwitchCurrentDirection(UInt32* aResultFlag);
	
	// Переключение словаря на направление, определяемое по строке
	ESldError SwitchDirection(const UInt16* aText, UInt32* aResultFlag);
	
	// Переключение словаря на определенное направление
	ESldError SwitchDirectionTo(ESldLanguage aLangFrom, EWordListTypeEnum aUsage, UInt32* aResultFlag, UInt32 alphabetType = EAlphabet_Standard);

	// Возвращает массив номеров стилей словаря, в которых есть различные варианты отображения ударений
	ESldError GetStylesWithStressVariants(Int32* aStyles, Int32* aCount);

	// Возвращает версию ядра, с которым был собран словарь
	ESldError GetEngineVersion(UInt32* aVersion) const;

	// Возвращает версию ядра, с которым был собран словарь для закрытой базы
	ESldError GetEngineVersionForced(ISDCFile *aFile, UInt32* aVersion);

	///Копирует строку без игнорируемых символов и в нижнем регистре.
	ESldError StrEffectiveCopyToLowerStr(UInt16* aDest, const UInt16* aSrc);

	// Получает вектор индексов озвучек по введенному тексту
	ESldError GetSoundIndexByText(const UInt16 *aText, CSldVector<Int32> & aSoundIndexes);
	// Проверяет есть ли у слова озвучка
	ESldError IsWordHasSound(const UInt16 *aText, UInt32 *aResultFlag);

	// Устанавливает нужную локализацию записей по переданному языковому коду
	ESldError SetLocalization(ESldLanguage aLocalizationLangCode);
	// Устанавливает нужную локализацию записей по переданному языковому коду и набору настроек
	ESldError SetLocalization(ESldLanguage aLocalizationLangCode, const UInt16* aLocalizationDetails);

	// Получает количество локализаций записей в текущем списке слов
	ESldError GetNumberOfLocalization(Int32 & aNumberOfLocalization);

	// Устанавливает нужную локализацию записей по номеру локализации в текущем списке слов
	ESldError SetLocalizationByIndex(const Int32 aIndex);

	// Получает информацию о локализации по ее номеру в текущем списке слов
	ESldError GetLocalizationInfoByIndex(const Int32 aIndex, UInt32 & aLocalizationCode, SldU16String & aLocalizationName);

	// Возвращает индекс текущей локализации
	ESldError GetCurrentLocalizationIndex(Int32 & aIndex);

	// Подготавливает объект для декодирования озвучки
	ESldError GetDecoderObject(UInt32 aSoundIndex, SldSpxDecoder *aDecoder, UInt32 aExternFlag = 0);

	// Проверяет сортированность текущего уровня иерархии
	ESldError IsCurrentLevelSorted(UInt32* aIsSorted);

	// Получает bitmap тематик блоков switch, используемых в словаре
	UInt32 GetSwitchThematicsBitmap() { return m_SwitchThematicsBitmap; };

	// Получает элемент истории для заданного слова в текущем списке слов
	ESldError GetHistoryElement(Int32 aIndex, CSldVector<CSldHistoryElement> & aHistoryElements);

	// Получает слово по элементу истории
	virtual ESldError GetWordByHistoryElement(CSldHistoryElement* aHistoryElement, ESldHistoryResult* aResultFlag, Int32* aListIndex, Int32* aGlobalIndex);

	// Обновляет элемент истории до текущего состояния базы
	virtual ESldError UpdateHistoryElement(CSldHistoryElement* aHistoryElement, ESldHistoryResult* aResultFlag);

	// Возвращает тип содержимого в контейнере
	ESlovoEdContainerDatabaseTypeEnum GetDatabaseType(void) const;

	// Проверяет полная это база или нет
	UInt32 IsInApp(void) const;

	// Возвращает количество результатов полнотекстового поиска
	ESldError GetFullTextResultCount(const UInt32 aListIndex, const UInt16* aRequest, Int32* aCount);
	// Возвращает количество результатов полнотекстового поиска для текущего списка
	ESldError GetFullTextResultCount(const UInt16* aRequest, Int32* aCount);

	// Проверяет наличие стилизованного текста для текущего варианта написания
	UInt32 HasStylizedVariantForCurrentWord(Int32 aVariantIndex);

	// Возвращает форматированный вариант написания для текущего слова
	virtual ESldError GetCurrentWordStylizedVariant(Int32 aVariantIndex, SplittedArticle & aArticle);

	// Добавляет пользовательский список
	ESldError AddCustomWordList(CSldCustomListControl* aListControl, Int32* aCustomListIndex, Int32 aBaseListIndex = SLD_DEFAULT_LIST_INDEX, UInt32 aUncoverHierarchy = 1);

	// Создает пользовательский список со словами, привязанными к текущему слову
	ESldError GetSubwordsList(CSldCustomListControl* aListControl, Int32* aCustomListIndex, const bool aUncoverHierarchy = false);

	// Возвращает информацию об элементарном объекте по его индексу
	ESldError GetAtomicObjectInfo(const Int32 aIndex, const TAtomicObjectInfo **aAtomicObject) const;

	// Возвращает информацию о использовании внешних баз
	ESldError GetExternContentInfo(TExternContentInfo (&aExtInf)[eExternContentTypeMax]) const;

	// Возвращает общее число ресурсов с информацией о внешних базах
	ESldError GetExternContentInfoCount(UInt32* aCount) const;

	// Возвращает индекс статьи
	ESldError GetTranslationIndex(Int32 aListIndex, Int32 aIndex, Int32 aVariantIndex, Int32* aArticleIndex);

	// Возвращает индекс статьи
	ESldError GetTranslationIndex(Int32 aIndex, Int32 aVariantIndex, Int32* aArticleIndex);

	// Получает дополнительную информацию о таблице сортировки
	ESldError GetCMPTableResourceInfo(UInt32 aIndex, TResourceInfo* aResInfo);

	// Получает дополнительную информацию об изображении
	ESldError GetPictureResourceInfo(UInt32 aIndex, TResourceInfo* aResInfo);
	// Получает дополнительную информацию об абстрактном ресурсе
	ESldError GetAbstractResourceInfo(UInt32 aResId, TResourceInfo* aResInfo);

	// Проверяет принадлежность базы к ЛЭУ
	ESldError IsETBDatabase(UInt8* aFlag) const;

	// Переходит по пути, соответсвующему заданному глобальному индексу в текущем списке слов
	ESldError GoToByGlobalIndex(const Int32 aGlobalIndex);
	// Переходит по пути, соответсвующему заданному глобальному индексу в заданном списке слов
	ESldError GoToByGlobalIndex(const Int32 aGlobalIndex, const Int32 aListIndex);

	// Индексирует все слова, содержащиеся в списке в соответсвии с заданным вариантом написания
	ESldError SortListByVariant(const Int32 aListIndex, const EListVariantTypeEnum aVariantType);

	// Получает изначальные индексы для полнотекстового поиска для текущего списка слов
	ESldError GetInitialWordIndexes(Int32 aLocalIndex, Int32* aInitialListIndex, Int32* aInitialWordIndex);
	// Получает изначальные индексы для полнотекстового поиска для заданного списка слов
	ESldError GetInitialWordIndexes(Int32 aListIndex, Int32 aLocalIndex, Int32* aInitialListIndex, Int32* aInitialWordIndex);

	// Возвращает "менеджер" структурированных метаданных
	CSldMetadataManager* GetMetadataManager() const { return m_MetadataManager.get(); }

	// Возвращает индекс листа со строчками структурированных метаданных
	Int32 GetMetadataStringsListIndex() const { return m_MetadataStringsListIndex; }

	// Возвращает "менеджер" css метаданных
	CSldCSSDataManager* GetCSSDataManager() const { return m_cssDataManager.get(); }

	// Возвращает индекс листа со строчками css метаданных
	Int32 GetCSSDataStringsListIndex() const { return m_cssDataStringsListIndex; }

	// Возвращает строку с javascript'ом ассоциированным со статьями
	ESldError GetArticlesJavaScript(SldU16String &aString);

	// Возвращает вектор индексов словаря для заданного слова в заданном списке слов
	virtual ESldError GetDictionaryIndexesByGlobalIndex(const Int32 aListIndex, const Int32 aGlobalIndex, CSldVector<Int32> & aDictIndexes);

	// Возвращает вектор индексов словаря для заданного слова в текущем списке слов
	ESldError GetDictionaryIndexesByGlobalIndex(const Int32 aGlobalIndex, CSldVector<Int32> & aDictIndexes);

	// Возвращает данные о слиянии
	virtual const TMergedMetaInfo* GetMergeInfo() const { return m_MergedDictInfo.MetaInfo; }

	// Возвращает индекс данного словаря в ядре слияния
	UInt32 GetMergedDictIndex() const { return m_MergedDictInfo.DictIndex; }

	// Устанавливает режим "мульти-поиска"
	// В этом режиме может вызываться несколько поисковых функций
	// Оболочка может отключить обработку  обычных eWordFoundCallbackStartSearch и eWordFoundCallbackStopSearch
	// до вызова eWordFoundCallbackStopMultiSearch
	ESldError StartMultiSearch() { return m_LayerAccess->WordFound(eWordFoundCallbackStartMultiSearch); }
	// Отключает режим "мульти-поиска"
	ESldError StopMultiSearch(){ return m_LayerAccess->WordFound(eWordFoundCallbackStopMultiSearch); }

	// Возвращает коллекцию ссылок, идущих в начале статьи
	ESldError GetArticleLinks(const Int32 aListIndex, const Int32 aGlobalIndex, SldLinksCollection & aLinks);

protected:
	// Инициализация членов по умолчанию
	virtual void Clear(void);

	// Добавляет список слов
	virtual ESldError AddList(sld2::UniquePtr<ISldList> aList, Int32 aListIndex);
	
	// Удаляет список слов
	virtual ESldError RemoveList(Int32 aListIndex);

	// Инициализируем систему защиты
	virtual ESldError InitRegistration(void);

	// Возвращает указатель на экземпляр объекта текущего списка слов
	ESldError GetCurrentWordlistPtr(ISldList **aListPtr);
	
	// Возвращает указатель на экземпляр объекта списка слов по номеру списка
	virtual ESldError GetWordList(Int32 aIndex, ISldList** aListPtr);

	// Разбивает китайское слово на слоги и возвращает вектор с индексами озвучки для конкретного слога, если способ разбиения найден 
	ESldError SearchChineseSound(const UInt16 *aText, Int32 aCount, CSldVector<Int32> & aSoundIndexes, UInt32* aResultFlag);

	// Получает информацию о картинке JPG
	ESldError GetJpgInfo(UInt8 *const apiData, UInt32 aiDataSize, Int32 *const apiWidth, Int32 *const apiHeight) const;

	// Получает готовую для проигрывания озвучку по индексу озвучки
	virtual ESldError PlaySoundByIndex(Int32 aSoundIndex, UInt8 aIsLast, UInt32* aStartPos, UInt32 aExternFlag, UInt8 aBeginPlay);

	// Удаляет списки определенного типа
	ESldError RemoveList(EWordListTypeEnum aListType, Int32 aListIndex = -1);

	// Добавляет в пользовательский список варианты написания из заданного списка
	ESldError UpdateVariantPropertyInCustomList(const Int32 aCheckedListIndex, const Int32 aCustomListIndex, CSldCustomListControl* aListControl);

	// Возвращает информацию об озвучке
	ESldError GetResourceInfo(UInt32 aResType, UInt32 aResIndex, TResourceInfo* aResInfo);	

	// Получает слово по элементу истории в заданном списке слов 
	ESldError GetWordByHistoryElementInList(CSldHistoryElement* aHistoryElement, const Int32 aListIndex, ESldHistoryResult* aResultFlag, Int32* aGlobalIndex);

	// Обновляет элемент истории до текущего состояния базы в заданном списке слов 
	ESldError UpdateHistoryElementInList(CSldHistoryElement* aHistoryElement, const Int32 aListIndex, ESldHistoryResult* aResultFlag);

	// Возвращает индекс первого листа заданного типа
	Int32 FindListIndex(UInt32 aType) const;

	// Загружает заголовок словарной базы
	ESldError LoadBaseHeader(ISDCFile *aFile);

	// Приводит индексы списка и слова к реальным записям, имеющим переводы
	ESldError GoToRealIndexes(Int32 & aRealListIndex, Int32 & aRealWordIndex);

	// XXX: non-copyable
	CSldDictionary(const CSldDictionary& aRef);
	CSldDictionary& operator=(const CSldDictionary& aRef);


	// Возвращает указатель на заголовок словаря
	virtual const TDictionaryHeader * GetDictionaryHeader() const { return m_Header; }

	// Возвращает указатель на данные, необходимые для работы прослойки
	virtual ISldLayerAccess * GetLayerAccess() { return m_LayerAccess; }

	// Возвращает индекс текущего списка слов
	virtual Int32 GetCurrentListIndex() const { return m_ListIndex; }

	// Возвращает индекс списка, который был текущим до операции поиска
	virtual Int32 GetPrevListIndex() const { return m_PrevListIndex; };

	// Возвращает класс чтения данных
	virtual CSDCReadMy * GetReadData() { return &m_data; }

	// Возвращает указатель на списки слов
	virtual ISldList ** GetLists() { return m_List; }

	// Возвращает ссылкку на класс сравнения строк
	virtual CSldCompare & GetCMP() { return m_CMP; }

	// Устанавливает данные о слиянии словарей
	ESldError SetMergeInfo(const TMergedMetaInfo *aInfo, UInt32 aDictIdx);

private:

	// Класс чтения данных
	CSDCReadMy				m_data;

	// Указатель на данные, необходимые для работы прослойки
	ISldLayerAccess 		*m_LayerAccess;

	// Указатель на заголовок словаря
	TDictionaryHeader		*m_Header;

	// Указатель на структуру с дополнительным данными
	TAdditionalInfo			*m_AdditionalInfo;

	// Строка с аннотацией к учебнику/базе
	UInt16					*m_Annotation;
	
	// Указатель на структуру с информацией о версии словарной базы
	TDictionaryVersionInfo	*m_DictionaryVersionInfo;

	// Массив указателей на информацию о списках слов
	CSldListInfo			**m_ListInfo;
	
	// Сравнение строк
	CSldCompare				m_CMP;

	// Списки слов
	ISldList				**m_List;
	
	// Количество списков слов (обычных и содержащих результаты поиска)
	Int32					m_ListCount;
	
	// Индекс списка, который был текущим до операции поиска
	Int32					m_PrevListIndex;

	// Номер текущего списка слов
	Int32					m_ListIndex;

	// Переводы
	sld2::UniquePtr<CSldArticles>	m_Articles;

	// Локализованные строчки словаря
	sld2::UniquePtr<CSldLocalizedString>	m_Strings;
	
	// Массив указателей на функции сборки озвучки
	FSoundBuilderMethodPtr	m_SoundBuilder[4];
	
	// Данные, необходимые для работы системы защиты
	TRegistrationData		m_RegistrationData;

	// Случайное число
	TRandomSeed				m_RandomSeed;

	// Bitmap тематик блоков switch, используемых в словаре
	UInt32					m_SwitchThematicsBitmap;

	sld2::UniquePtr<CSldMorphology> m_MorphologyManager;

	// Структура с информацией об атомарных объектах
	TAtomicObjectInfo		*m_AtomicInfo;

	// Класс работы со структурированными метаданными
	sld2::UniquePtr<CSldMetadataManager> m_MetadataManager;

	// Индекс листа содержащего строки структурированных метаданных
	Int32					m_MetadataStringsListIndex;

	// Класс работы с css метаданными
	sld2::UniquePtr<CSldCSSDataManager>	m_cssDataManager;

	// Индекс листа содержащего строки css метаданных
	Int32					m_cssDataStringsListIndex;

	// Данные о смреженых словарях
	TMergedDictInfo			m_MergedDictInfo;
};

#endif
