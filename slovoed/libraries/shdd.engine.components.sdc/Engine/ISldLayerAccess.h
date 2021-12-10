#ifndef _I_SLD_LAYER_ACCESS_H_
#define _I_SLD_LAYER_ACCESS_H_

#include "SldError.h"
#include "SldTypes.h"
#include "SldSDCReadMy.h"


// Class preliminary declaration.
class CSldDictionary;

/***********************************************************************
*	This class is responsible for the interaction between the kernel and the program.
************************************************************************/
class ISldLayerAccess
{
public:

	// Destructor
	virtual ~ISldLayerAccess(void) {};

	/***********************************************************************
	* The method that assembles the translation.
	*
	* @param aDictionary	- a pointer to an instance of the dictionary,
	*                         which stores all the data for the article
	* @param aText			- piece of text
	* @param aTree			- style number for formatting the transferred piece
	*                         of text or flags of the beginning / end of the article
	*
	* @return error code
	************************************************************************/
	virtual ESldError BuildTranslationRight(const CSldDictionary *aDictionary, const UInt16 *aText, ESldTranslationModeType aTree) = 0;

	/***********************************************************************
	* The method that assembles the message that the dictionary is not registered.
	*
	* @param aDictionary	- a pointer to an instance of the dictionary,
	*                         which stores all the data for the article
	* @param aText			- piece of text
	* @param aTree			- style number for formatting the transferred piece
	*                         of text or flags of the beginning / end of the article
	*
	* @return error code
	************************************************************************/
	virtual ESldError BuildTranslationWrong(const CSldDictionary *aDictionary, const UInt16 *aText, ESldTranslationModeType aTree) = 0;

	/***********************************************************************
	* The method performs actions if a word was found when searching by a pattern.
	* In addition, this method is called periodically, even if nothing was
	* found, so that the shell can gracefully stop the search.
	*
	* @param aText	- name of the found word. If NULL, then this is a call to
	*                 find out if the shell wants to stop searching.
	* @param aIndex	- the index of the word found. If MAX_UINT_VALUE is a call
	*                 to see if the shell wants to stop searching.
	*
	* @return error code
	************************************************************************/
	virtual ESldError WordFound(const ESldWordFoundCallbackType aCallbackType, const UInt32 aIndex = 0) = 0;
	
	/***********************************************************************
	* Returns the platform identifier
	*
	* @return platform identifier
	************************************************************************/
	virtual const UInt16 * GetPlatformID() = 0;

	/** ********************************************************************
	* Saves registration data
	*
	* @param aDictID	- the identifier of the dictionary for which the data is being saved.
	* @param aData		- a pointer to the data to be stored.
	* @param aSize		- the amount of data to save.
	*
	* @return error code
	************************************************************************/
	virtual ESldError SaveSerialData(UInt32 aDictID, const UInt8 *aData, UInt32 aSize) = 0;

	/** ********************************************************************
	* Считываем регистрационные данные
	*
	* @param aDictID	- идентификатор словаря для которого считываются данные.
	* @param aData		- указатель на память куда нужно будет загрузить данные.
	*						Если NULL, значит, что в aSize нужно будет поместить 
	*						размер необходимой памяти. 
	* @param aSize		- указатель на переменную с количеством данных для сохранения, 
	*						при выходе из функции сюда помещается размер который был необходим в байтах.
	*
	* @return error code
	*
	*
	* Шаблон использования выглядит так:
	* <code>
	*	UInt32 size;
	*	LoadSerialData(DictID, NULL, &size);
	*	Int8 *p = new Int8[size];
	*	LoadSerialData(DictID, p, &size);
	* </code>
	************************************************************************/
	virtual ESldError LoadSerialData(UInt32 aDictID, UInt8 *aData, UInt32 *aSize) = 0;

	/*************************************************************************************************
	Метод, который производит сборку озвучки
	**************************************************************************************************
	@param aBlockPtr - указатель на память, откуда считываются готовые для воспроизведения данные.
	@param aBlockSize - размер данных, который был передан в данную итерацию.
	@param aPreviousSize - количество данных, которые были переданы на предыдущих итерациях.
	@param aFrequency - частота дискретизации данных.
	@param aFinishFlag - флаг, который сообщает, закончилось декодирование звука или нет. Может принимать 
							следующие значения:
							#SLD_SOUND_FLAG_START - начало декодирования, никаких реальных данных не передается.
							#SLD_SOUND_FLAG_CONTINUE - продолжается декодирование.
							#SLD_SOUND_FLAG_FINISH - конец декодирования, никаких реальных данных не передается.
													 Если нужно произвести какие-либо действия по окончанию процесса
													 декодирования звука, тогда их нужно проводить в данный момент.
	@param eOK - в случае успеха, иначе ошибку.
	*/
	virtual ESldError BuildSoundRight(const UInt8 *aBlockPtr, UInt32 aBlockSize, UInt32 aPreviousSize, UInt32 aFrequency, UInt32 aFinishFlag) = 0;
	
	/*************************************************************************************************
	Метод, который производит сборку озвучки в случае, когда озвучка не зарегистрирована
	**************************************************************************************************
	@param aBlockPtr - указатель на память, откуда считываются готовые для воспроизведения данные.
	@param aBlockSize - размер данных, который был передан в данную итерацию.
	@param aPreviousSize - количество данных, которые были переданы на предыдущих итерациях.
	@param aFrequency - частота дискретизации данных.
	@param aFinishFlag - флаг, который сообщает, закончилось декодирование звука или нет. Может принимать 
							следующие значения:
							#SLD_SOUND_FLAG_START - начало декодирования, никаких реальных данных не передается.
							#SLD_SOUND_FLAG_CONTINUE - продолжается декодирование.
							#SLD_SOUND_FLAG_FINISH - конец декодирования, никаких реальных данных не передается.
													 Если нужно произвести какие-либо действия по окончанию процесса
													 декодирования звука, тогда их нужно проводить в данный момент.
	@param eOK - в случае успеха, иначе ошибку.
	*/
	virtual ESldError BuildSoundWrong(const UInt8 *aBlockPtr, UInt32 aBlockSize, UInt32 aPreviousSize, UInt32 aFrequency, UInt32 aFinishFlag) = 0;
	
	
	/***********************************************************************
	* Загружает озвучку из внешнего источника по индексу озвучки (не из sdc)
	* Эта функция вызывается ядром, когда ядру необходимо получить озвучку извне
	*
	* @param aSoundIndex	- индекс озвучки
	* @param aDataPtr		- указатель, по которому нужно записать указатель на память с загруженными данными
	* @param aDataSize		- указатель на переменную, в которую нужно сохранить размер в байтах загруженных данных
	*
	* @return error code
	************************************************************************/
	virtual ESldError LoadSoundByIndex(Int32 aSoundIndex, const UInt8** aDataPtr, UInt32* aDataSize) = 0;
	
	/***********************************************************************
	* Загружает картинку из внешнего источника по индексу картинки (не из sdc)
	* Эта функция вызывается ядром, когда ядру необходимо получить картинку извне
	*
	* @param aImageIndex	- индекс картинки
	* @param aDataPtr		- указатель, по которому нужно записать указатель на память с загруженными данными
	* @param aDataSize		- указатель на переменную, в которую нужно сохранить размер в байтах загруженных данных
	*
	* @return error code
	************************************************************************/
	virtual ESldError LoadImageByIndex(Int32 aImageIndex, const UInt8** aDataPtr, UInt32* aDataSize) = 0;
};


// Описание типа метода, осуществляющего сборку перевода
/**
	Данный метод производит сборку перевода или сообщения о том, что словарь не зарегистрирован

	@param aArticles	- указатель на экземпляр словаря, в котором хранятся все данные по статье
	@param aText		- кусок текста
	@param aTree		- номер стиля для форматирования переданного куска текста или флаги начала/конца статьи
	
	@return error code
*/
typedef ESldError (ISldLayerAccess::*FTranslationBuilderMethodPtr)(const CSldDictionary *aDictionary, const UInt16 *aText, ESldTranslationModeType aTree);



// Описание типа метода, осуществляющего сборку озвучки
/**
	Данный метод производит сборку озвучки либо тонового сигнала в случае, если словарь не зарегистрирован

	@param aBlockPtr		- указатель на память, откуда считываются готовые для воспроизведения данные
	@param aBlockSize		- размер данных, который был передан в данную итерацию
	@param aPreviousSize	- количество данных, которые были переданы на предыдущих итерациях
	@param aFrequency		- частота дискретизации данных
	@param aFinishFlag		- флаг, который сообщает, закончилось декодирование звука или нет
	
	@return error code
*/
typedef ESldError (ISldLayerAccess::*FSoundBuilderMethodPtr)(const UInt8 *aBlockPtr, UInt32 aBlockSize, UInt32 aPreviousSize, UInt32 aFrequency, UInt32 aFinishFlag);

#endif
