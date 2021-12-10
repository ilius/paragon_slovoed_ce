from distutils.core import setup, Extension

# python3 -c 'import sdc_engine'

sources = [
	"PyInit.cpp",

	"SldSDCReadMy.cpp", # SldSDCReadMy.h is included in the rest of following files
	"SldCatalog.cpp",
	"SldListLocalizedString.cpp",
	"SldLocalizedString.cpp",
	"SldMorphology.cpp",
	"SldBitInput.cpp",
	"SldStyleInfo.cpp",
	"SldSerialNumber.cpp",
	"SldSymbolsTable.cpp",
	"SldStringStore.cpp",
	"SldCSSDataManager.cpp",
	"SldMetadataManager.cpp",
	"SldArticles.cpp",
	"SldCompare.cpp",
	"SldIntFormatter.cpp",

	"SldDictionary.cpp",
	"SldSearchList.cpp",
	"SldCustomList.cpp",
	"SldList.cpp",
	"SldSimpleSortedList.cpp",
	"SldCustomListControl.cpp",
	"SldMp3Decoder.cpp",
	"SldSearchWordResult.cpp",
	"SldLogicalExpression.cpp",
	"SldHistory.cpp",
	"SldSpeexDecoder.cpp",
	"SldTools.cpp",
	"SldInputText.cpp",
	"SldInputBase.cpp",
	"SldMetadataParser.cpp",
	"SldVideoItem.cpp",
	"SldImageAreaItem.cpp",
	"SldImageItem.cpp",
	"SldVideoElement.cpp",
	"SldLogicalExpressionImplementation.cpp",
	"SldListInfo.cpp",
	"SldIndexes.cpp",
	"SldOggDecoder.cpp",
	"SldTypes.cpp",
	"SldSpeexSinTable.cpp",
	"SldWavDecoder.cpp",
	"SldMergedList.cpp",
	"SDC_CRC.cpp",

	"Morphology/MorphoData.cpp",
	"Morphology/MorphoData_v1.cpp",
	"Morphology/MorphoData_v2.cpp",
	"Morphology/MorphoData_v3.cpp",
	"Morphology/LanguageSpecific_v1.cpp",
	"Morphology/LanguageSpecific_v2.cpp",
	"Morphology/LastCharMap_v1.cpp",
	"Morphology/LastCharMap_v2.cpp",
	"Morphology/IMorphoData.cpp",
	"Morphology/WordSet_v1.cpp",
	"Morphology/WordSet_v2.cpp",

	"Speex/modes.c",
	"Speex/sb_celp.c",
	"Speex/vbr.c",
	"Speex/quant_lsp.c",
	"Speex/cb_search.c",
	"Speex/nb_celp.c",
	"Speex/ltp.c",

	"Speex/bits.c",
	"Speex/exc_10_16_table.c",
	"Speex/exc_10_32_table.c",
	"Speex/exc_20_32_table.c",
	"Speex/exc_5_256_table.c",
	"Speex/exc_5_64_table.c",
	"Speex/exc_8_128_table.c",
	"Speex/filters.c",
	"Speex/gain_table.c",
	"Speex/gain_table_lbr.c",
	"Speex/hexc_10_32_table.c",
	"Speex/hexc_table.c",
	"Speex/high_lsp_tables.c",
	"Speex/lpc.c",
	"Speex/lsp.c",
	"Speex/lsp_tables_nb.c",
	"Speex/math_approx.c",
	"Speex/misc.c",
	"Speex/speex_callbacks.c",
	"Speex/speex_header.c",
	"Speex/stereo.c",
	"Speex/vq.c",

]


module1 = Extension(
	"sdc_engine",
	define_macros=[
		# ('MAJOR_VERSION', '1'),
		# ('MINOR_VERSION', '0')
	],
	include_dirs=[
		"/usr/local/include",
		"/usr/local/lib/python3.9/dist-packages/pybind11/include",
	],
	libraries=[],
	library_dirs=['/usr/local/lib'],
	sources=sources,
)

setup(
	name = "sdc_engine",
	version = "1.0",
	description = "sdc_engine",
	ext_modules = [module1]
)
