/*                                                                   StockList.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                  STOCK LIST                                  #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>

//****************************************************************************//
//      Main menu labels                                                      //
//****************************************************************************//

//============================================================================//
//      "File" menu labels                                                    //
//============================================================================//
# define	MENU_FILE_NEW				"_New"				// "New" menu button
# define	MENU_FILE_OPEN				"_Open"				// "Open" menu button
# define	MENU_FILE_SAVE				"_Save"				// "Save" menu button
# define	MENU_FILE_SAVE_AS			"Save _As..."		// "Save As" menu button
# define	MENU_FILE_IMPORT			"_Import"			// "Import" menu button
# define	MENU_FILE_EXPORT			"E_xport"			// "Export" menu button
# define	MENU_FILE_PROPERTIES		"_Properties"		// "Properties" menu button
# define	MENU_FILE_CLOSE				"_Close"			// "Close" menu button
# define	MENU_FILE_QUIT				"_Quit"				// "Quit" menu button

//============================================================================//
//      "Edit" menu labels                                                    //
//============================================================================//
# define	MENU_EDIT_INSERT			"_Insert"			// "Insert" menu button
# define	MENU_EDIT_REMOVE			"_Remove"			// "Remove" menu button
# define	MENU_EDIT_SELECT_ALL		"Select _All"		// "Select All" menu button
# define	MENU_EDIT_INVERT_SELECTION	"I_nvert selection"	// "Invert selection" menu button
# define	MENU_EDIT_UNSELECT_ALL		"_Unselect All"		// "Unselect All" menu button
# define	MENU_EDIT_FIND				"_Find"				// "Find" menu button
# define	MENU_EDIT_SORT				"_Sort"				// "Sort" menu button

//============================================================================//
//      "Stocks" menu labels                                                  //
//============================================================================//
# define	MENU_STOCKS_INFO			"_Info"				// "Info" menu button
# define	MENU_STOCKS_CHECK			"_Check"			// "Check" menu button
# define	MENU_STOCKS_SYNC			"_Sync"				// "Sync" menu button
# define	MENU_STOCKS_ANALYZE			"Analy_ze"			// "Analyze" menu button

//============================================================================//
//      "Quotes" menu labels                                                  //
//============================================================================//
# define	MENU_QUOTES_VIEW			"_View"				// "View" menu button
# define	MENU_QUOTES_EDIT			"_Edit"				// "Edit" menu button

//============================================================================//
//      "Help" menu labels                                                    //
//============================================================================//
# define	MENU_HELP_ABOUT				"_About"			// "About" menu button

//****************************************************************************//
//      Tool button labels                                                    //
//****************************************************************************//
# define	STOCK_TOOL_NEW			"New"		// "New" tool bar button
# define	STOCK_TOOL_OPEN			"Open"		// "Open" tool bar button
# define	STOCK_TOOL_SAVE			"Save"		// "Save" tool bar button
# define	STOCK_TOOL_IMPORT		"Import"	// "Import" tool bar button
# define	STOCK_TOOL_EXPORT		"Export"	// "Export" tool bar button
# define	STOCK_TOOL_INSERT		"Insert"	// "Insert" tool bar button
# define	STOCK_TOOL_REMOVE		"Remove"	// "Remove" tool bar button
# define	STOCK_TOOL_ALL			"All"		// "All" tool bar button
# define	STOCK_TOOL_NONE			"None"		// "None" tool bar button
# define	STOCK_TOOL_FIND			"Find"		// "Find" tool bar button
# define	STOCK_TOOL_CHECK		"Check"		// "Check" tool bar button
# define	STOCK_TOOL_SYNC			"Sync"		// "Sync" tool bar button
# define	STOCK_TOOL_ANALYZE		"Analyze"	// "Analyze" tool bar button
# define	STOCK_TOOL_QUOTES		"Quotes"	// "Quotes" tool bar button
# define	STOCK_TOOL_INFO			"Info"		// "Info" tool bar button
# define	STOCK_TOOL_QUIT			"Quit"		// "Quit" tool bar button

//****************************************************************************//
//      Stock list constants                                                  //
//****************************************************************************//
# define	STOCK_COLUMNS			7			// Count of columns in stock list

//============================================================================//
//      Field ids                                                             //
//============================================================================//
# define	STOCK_TICKER_ID			0			// Stock ticker field id
# define	STOCK_NAME_ID			1			// Stock name field id
# define	STOCK_COUNTRY_ID		2			// Stock country field id
# define	STOCK_SECTOR_ID			3			// Stock sector field id
# define	STOCK_INDUSTRY_ID		4			// Stock industry field id
# define	STOCK_URL_ID			5			// Stock URL field id
# define	STOCK_CHECK_ID			6			// Check button field id

//============================================================================//
//      Field names                                                           //
//============================================================================//
# define	STOCK_TICKER_LABEL		"Ticker"	// Stock ticker field label
# define	STOCK_NAME_LABEL		"Name"		// Stock name field label
# define	STOCK_COUNTRY_LABEL		"Country"	// Stock country field label
# define	STOCK_SECTOR_LABEL		"Sector"	// Stock sector field label
# define	STOCK_INDUSTRY_LABEL	"Industry"	// Stock industry field label
# define	STOCK_URL_LABEL			"Url"		// Stock URL field label
# define	STOCK_CHECK_LABEL		""			// Check button field label

//****************************************************************************//
//      Function prototypes                                                   //
//****************************************************************************//
gboolean CheckField (const gchar *string, const gchar *field, GError **error);
gboolean UnselectAllStocks (void);
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
