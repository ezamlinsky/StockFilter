/*                                                                 StockList.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                  STOCK LIST                                  #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# include	<Common.h>
# include	<Stocks.h>
# include	<Quotes.h>
# include	<StockList.h>
# include	<QuoteList.h>
# include	<CheckList.h>
# include	<SyncList.h>
# include	<AnalyzeList.h>

//****************************************************************************//
//      Internal constants                                                    //
//****************************************************************************//
# define	ENTRY_MAX_CHARS		60			// Max length of text entry

//============================================================================//
//      Quotes count range                                                    //
//============================================================================//
# define	QUOTES_DIGITS		0			// Amount of decimal digits quotes count have
# define	QUOTES_MIN			1			// Min quotes count value
# define	QUOTES_MAX			1024		// Max quotes count value
# define	QUOTES_STEP			1			// Step increment for quotes count change
# define	QUOTES_PAGE			10			// Page increment for quotes count change
# define	QUOTES_DEFAULT		260			// Default quotes count

//============================================================================//
//      Stock liquidity range                                                 //
//============================================================================//
# define	LIQUIDITY_DIGITS	0			// Amount of decimal digits liquidity have
# define	LIQUIDITY_MIN		0			// Min stock liquidity value
# define	LIQUIDITY_MAX		1000000000	// Max stock liquidity value
# define	LIQUIDITY_STEP		100			// Step increment for stock liquidity change
# define	LIQUIDITY_PAGE		1000		// Page increment for stock liquidity change
# define	LIQUIDITY_DEFAULT	100000		// Default stock liquidity

//============================================================================//
//      Stock volatility range                                                //
//============================================================================//
# define	VOLATILITY_DIGITS	2			// Amount of decimal digits volatility have
# define	VOLATILITY_MIN		0.01		// Min stock volatility value
# define	VOLATILITY_MAX		100			// Max stock volatility value
# define	VOLATILITY_STEP		0.01		// Step increment for stock volatility change
# define	VOLATILITY_PAGE		0.1			// Page increment for stock volatility change
# define	VOLATILITY_DEFAULT	1.0			// Default stock volatility

//============================================================================//
//      Stock price range                                                     //
//============================================================================//
# define	PRICE_DIGITS		2			// Amount of decimal digits price have
# define	PRICE_MIN			0.01		// Min stock price value
# define	PRICE_MAX			1000000		// Max stock price value
# define	PRICE_STEP			0.01		// Step increment for stock price change
# define	PRICE_PAGE			1.00		// Page increment for stock price change
# define	PRICE_DEFAULT		5.00		// Default stock price

//****************************************************************************//
//      Label list structure                                                  //
//****************************************************************************//
struct LabelArray
{
	GArray		*array;						// Labels array
	gboolean	state;						// New button state
};

//****************************************************************************//
//      Global objects                                                        //
//****************************************************************************//
GtkWidget		*window;					// Main window
GtkWidget		*filemenu;					// File menu
GtkWidget		*editmenu;					// Edit menu
GtkWidget		*stocksmenu;				// Stocks menu
GtkWidget		*quotesmenu;				// Quotes menu
GtkWidget		*helpmenu;					// Help menu
GtkWidget		*toolbar;					// Tool bar
GtkWidget		*treeview;					// Tree view
GtkWidget		*statusbar;					// Status bar

//****************************************************************************//
//      Global variables                                                      //
//****************************************************************************//
guint			menuid;						// Status bar context identifier for menu messages
guint			fileid;						// Status bar context identifier for file messages
gchar			*file_name = NULL;			// Opened file name
gchar			*time_zone = NULL;			// Time zone file name
gboolean		saved = TRUE;				// Save document state

//****************************************************************************//
//      Local objects                                                         //
//****************************************************************************//
GtkAccelGroup	*accelgroup;				// Accelerator group
GdkPixbuf		*logo;						// Logo image
Stocks			stocks;						// Stock list
Quotes			quotes;						// Quotes list

//****************************************************************************//
//      Local variables                                                       //
//****************************************************************************//

// Button labels which change sensitivity when opening and closing stock list
const gchar* FileMenuOpenClose[] = {
	MENU_FILE_SAVE,
	MENU_FILE_SAVE_AS,
	MENU_FILE_IMPORT,
	MENU_FILE_EXPORT,
	MENU_FILE_PROPERTIES,
	MENU_FILE_CLOSE
};
const gchar* EditMenuOpenClose[] = {
	MENU_EDIT_INSERT,
	MENU_EDIT_REMOVE,
	MENU_EDIT_SELECT_ALL,
	MENU_EDIT_INVERT_SELECTION,
	MENU_EDIT_UNSELECT_ALL,
	MENU_EDIT_FIND,
	MENU_EDIT_SORT
};
const gchar* StocksMenuOpenClose[] = {
	MENU_STOCKS_INFO,
	MENU_STOCKS_CHECK,
	MENU_STOCKS_SYNC,
	MENU_STOCKS_ANALYZE
};
const gchar* QuotesMenuOpenClose[] = {
	MENU_QUOTES_VIEW,
	MENU_QUOTES_EDIT
};
const gchar* ToolBarOpenClose[] = {
	STOCK_TOOL_SAVE,
	STOCK_TOOL_IMPORT,
	STOCK_TOOL_EXPORT,
	STOCK_TOOL_INSERT,
	STOCK_TOOL_REMOVE,
	STOCK_TOOL_ALL,
	STOCK_TOOL_NONE,
	STOCK_TOOL_FIND,
	STOCK_TOOL_CHECK,
	STOCK_TOOL_SYNC,
	STOCK_TOOL_ANALYZE,
	STOCK_TOOL_QUOTES,
	STOCK_TOOL_INFO
};

// Button labels which change sensitivity when saving and modifying stock list
const gchar* FileMenuSaveUnsave[] = {
	MENU_FILE_SAVE
};
const gchar* ToolBarSaveUnsave[] = {
	STOCK_TOOL_SAVE
};

// Button labels which change sensitivity when changing selected stock
const gchar* StocksMenuSelectUnselect[] = {
	MENU_STOCKS_INFO
};
const gchar* QuotesMenuSelectUnselect[] = {
	MENU_QUOTES_VIEW,
	MENU_QUOTES_EDIT
};
const gchar* ToolBarSelectUnselect[] = {
	STOCK_TOOL_QUOTES,
	STOCK_TOOL_INFO
};

//****************************************************************************//
//      Global functions                                                      //
//****************************************************************************//

//============================================================================//
//      Check stock field                                                     //
//============================================================================//
gboolean CheckField (const gchar *string, const gchar *field, GError **error)
{
	// Check if field is empty
	if (g_utf8_strlen (string, -1) == 0)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Stock %s is empty", field);
		return FALSE;
	}

	// Normal exit
	return TRUE;
}

//****************************************************************************//
//      Internal functions                                                    //
//****************************************************************************//
static gboolean SaveStocks (void);
static gboolean SaveStocksAs (void);

//============================================================================//
//      Check if ticker unique                                                //
//============================================================================//
static gboolean IsTickerUnique (GtkTreeModel *model, const gchar *value, GError **error)
{
	// Get iterator position
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	{
		// Iterate through all elements
		do {
			// Get ticker value
			gchar *ticker;
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, STOCK_TICKER_ID, &ticker, -1);

			// If value is already into the list
			if (g_utf8_collate (ticker, value) == 0)
			{
				// Set error message
				g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Stock '%s' already exists", value);
				return FALSE;
			}

			// Free temporary string buffer
			g_free (ticker);

			// Change iterator position to next element
		} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
	}

	// Normal exit
	return TRUE;
}

//============================================================================//
//      Set sort column of stock list                                         //
//============================================================================//
static void SetSortColumn (GtkTreeView *treeview, gint column, GtkSortType order)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Set search column for immediate search
		gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), column);

		// Sort stocks by chosen field
		gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), column, order);
	}
}

//============================================================================//
//      Ask to save stock list                                                //
//============================================================================//
static void AskToSaveStockList (void)
{
	// Create message dialog window
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, "Stock list is not saved");

	// Set dialog message
	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "You should save stock list before processing.");

	// Run dialog window
	gtk_dialog_run (GTK_DIALOG (dialog));

	// Destroy dialog widget
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

//============================================================================//
//      Save stock list changes                                               //
//============================================================================//
static gboolean SaveChanges (void)
{
	// Operation status
	gboolean status = TRUE;

	// Check if stocks is not saved
	if (!saved)
	{
		// Create message dialog window
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "Stock list was changed");

		// Set dialog message
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "Would you like to save it?");

		// Set default dialog button
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);

		// Run dialog window
		gint response = gtk_dialog_run (GTK_DIALOG (dialog));

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (dialog));

		// Check if user chose to save stock list
		if (response == GTK_RESPONSE_YES)
		{
			// Try to save stock list
			if (!SaveStocks ())
			{
				// If can not save changes, then return fail status
				return FALSE;
			}
		}
	}

	// Return operation status
	return status;
}

//============================================================================//
//      Change menu buttons sensitivity                                       //
//============================================================================//
static void ChangeMenuButtonsSensitivity (GtkWidget *widget, gpointer data)
{
	// Check if widget is menu item
	if (GTK_IS_MENU_ITEM (widget) && !GTK_IS_SEPARATOR_MENU_ITEM (widget))
	{
		// Get menu item label
		const gchar *label = gtk_menu_item_get_label (GTK_MENU_ITEM (widget));

		// Convert data pointer
		LabelArray *larray = reinterpret_cast <LabelArray*> (data);

		// Get label array properties
		GArray *array = larray -> array;
		guint size = array -> len;
		gboolean state = larray -> state;

		// Iterate through all elements
		for (guint i = 0; i < size; i++)
		{
			// Check if menu label match to array element
			if (g_utf8_collate (label, g_array_index (array, gchar*, i)) == 0)
			{
				// Change sensitivity state of the menu item
				gtk_widget_set_sensitive (GTK_WIDGET (widget), state);
			}
		}
	}
}

//============================================================================//
//      Change tool buttons sensitivity                                       //
//============================================================================//
static void ChangeToolButtonsSensitivity (GtkWidget *widget, gpointer data)
{
	// Check if widget is tool button
	if (GTK_IS_TOOL_BUTTON (widget))
	{
		// Get tool button label
		const gchar *label = gtk_tool_button_get_label (GTK_TOOL_BUTTON (widget));

		// Convert data pointer
		LabelArray *larray = reinterpret_cast <LabelArray*> (data);

		// Get label array properties
		GArray *array = larray -> array;
		guint size = array -> len;
		gboolean state = larray -> state;

		// Iterate through all elements
		for (guint i = 0; i < size; i++)
		{
			// Check if button label match to array element
			if (g_utf8_collate (label, g_array_index (array, gchar*, i)) == 0)
			{
				// Change sensitivity state of the button
				gtk_widget_set_sensitive (GTK_WIDGET (widget), state);
			}
		}
	}
}

//****************************************************************************//
//      Program state functions                                               //
//****************************************************************************//

//============================================================================//
//      Change document state                                                 //
//============================================================================//
static void ChangeDocumentState (gchar *fname, gchar *tzone, gboolean state)
{
	// Change stock list file name
	g_free (file_name);
	file_name = g_strdup (fname);

	// Change stock list time zone
	g_free (time_zone);
	time_zone = g_strdup (tzone);

	// Change tree view sensitivity state
	gtk_widget_set_sensitive (GTK_WIDGET (treeview), state);

	// Create arrays of elements to change their sensitivity
	GArray *file_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));
	GArray *edit_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));
	GArray *stocks_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));
	GArray *quotes_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));
	GArray *tool_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));

	// Set array elements
	g_array_append_vals (file_array, FileMenuOpenClose, sizeof (FileMenuOpenClose) / sizeof (const gchar*));
	g_array_append_vals (edit_array, EditMenuOpenClose, sizeof (EditMenuOpenClose) / sizeof (const gchar*));
	g_array_append_vals (stocks_array, StocksMenuOpenClose, sizeof (StocksMenuOpenClose) / sizeof (const gchar*));
	g_array_append_vals (quotes_array, QuotesMenuOpenClose, sizeof (QuotesMenuOpenClose) / sizeof (const gchar*));
	g_array_append_vals (tool_array, ToolBarOpenClose, sizeof (ToolBarOpenClose) / sizeof (const gchar*));

	// Set label arrays
	LabelArray FileArray = {file_array, state};
	LabelArray EditArray = {edit_array, state};
	LabelArray StocksArray = {stocks_array, state};
	LabelArray QuotesArray = {quotes_array, state};
	LabelArray ToolArray = {tool_array, state};

	// Change elements sensitivity
	gtk_container_foreach (GTK_CONTAINER (filemenu), ChangeMenuButtonsSensitivity, &FileArray);
	gtk_container_foreach (GTK_CONTAINER (editmenu), ChangeMenuButtonsSensitivity, &EditArray);
	gtk_container_foreach (GTK_CONTAINER (stocksmenu), ChangeMenuButtonsSensitivity, &StocksArray);
	gtk_container_foreach (GTK_CONTAINER (quotesmenu), ChangeMenuButtonsSensitivity, &QuotesArray);
	gtk_container_foreach (GTK_CONTAINER (toolbar), ChangeToolButtonsSensitivity, &ToolArray);

	// Release array objects
	g_array_free (file_array, TRUE);
	g_array_free (edit_array, TRUE);
	g_array_free (stocks_array, TRUE);
	g_array_free (quotes_array, TRUE);
	g_array_free (tool_array, TRUE);
}

//============================================================================//
//      Change save state                                                     //
//============================================================================//
static void ChangeSaveState (gboolean state)
{
	// Change document saved flag
	saved = state;

	// Create arrays of elements to change their sensitivity
	GArray *file_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));
	GArray *tool_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));

	// Set array elements
	g_array_append_vals (file_array, FileMenuSaveUnsave, sizeof (FileMenuSaveUnsave) / sizeof (const gchar*));
	g_array_append_vals (tool_array, ToolBarSaveUnsave, sizeof (ToolBarSaveUnsave) / sizeof (const gchar*));

	// Set label arrays
	LabelArray FileArray = {file_array, !state};
	LabelArray ToolArray = {tool_array, !state};

	// Make elements insesitive
	gtk_container_foreach (GTK_CONTAINER (filemenu), ChangeMenuButtonsSensitivity, &FileArray);
	gtk_container_foreach (GTK_CONTAINER (toolbar), ChangeToolButtonsSensitivity, &ToolArray);

	// Release array objects
	g_array_free (file_array, TRUE);
	g_array_free (tool_array, TRUE);
}

//============================================================================//
//      Change selection state                                                //
//============================================================================//
static void ChangeSelectionState (gboolean state)
{
	// Create arrays of elements to change their sensitivity
	GArray *stocks_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));
	GArray *quotes_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));
	GArray *tool_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));

	// Set array elements
	g_array_append_vals (stocks_array, StocksMenuSelectUnselect, sizeof (StocksMenuSelectUnselect) / sizeof (const gchar*));
	g_array_append_vals (quotes_array, QuotesMenuSelectUnselect, sizeof (QuotesMenuSelectUnselect) / sizeof (const gchar*));
	g_array_append_vals (tool_array, ToolBarSelectUnselect, sizeof (ToolBarSelectUnselect) / sizeof (const gchar*));

	// Set label arrays
	LabelArray StocksArray = {stocks_array, state};
	LabelArray QuotesArray = {quotes_array, state};
	LabelArray ToolArray = {tool_array, state};

	// Make elements insesitive
	gtk_container_foreach (GTK_CONTAINER (stocksmenu), ChangeMenuButtonsSensitivity, &StocksArray);
	gtk_container_foreach (GTK_CONTAINER (quotesmenu), ChangeMenuButtonsSensitivity, &QuotesArray);
	gtk_container_foreach (GTK_CONTAINER (toolbar), ChangeToolButtonsSensitivity, &ToolArray);

	// Release array objects
	g_array_free (stocks_array, TRUE);
	g_array_free (quotes_array, TRUE);
	g_array_free (tool_array, TRUE);
}

//****************************************************************************//
//      Signal handler for "select" menu signal                               //
//****************************************************************************//
static void MenuSelect (GtkMenuItem *menuitem, gpointer data)
{
	// Push new message into status bar stack
	gtk_statusbar_push (GTK_STATUSBAR (statusbar), menuid, reinterpret_cast <const gchar*> (data));
}

//****************************************************************************//
//      Signal handler for "deselect" menu signal                             //
//****************************************************************************//
static void MenuDeselect (GtkMenuItem *menuitem, gpointer data)
{
	// Pop message from the top of status bar stack
	gtk_statusbar_pop (GTK_STATUSBAR (statusbar), menuid);
}

//****************************************************************************//
//      Signal handler for "New" menu button                                  //
//****************************************************************************//
static gboolean NewStocks (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// If file is changed, then ask to save changes before continue
		if (!SaveChanges ())
		{
			// If can not save changes, then return fail status
			return FALSE;
		}
	}

	// Create dialog window
	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Select time zone for new stock list", GTK_WINDOW (window), GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Select", GTK_RESPONSE_ACCEPT, NULL);

	// Set file chooser properties
	gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (dialog), FALSE);
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), TRUE);
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), FALSE);
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "/usr/share/zoneinfo/");

	// Set default dialog button
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	// Run dialog window
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		// Get chosen time zone file name
		gchar *tzone = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		// Create new stock list
		stocks.NewList (tzone);

		// Set main window title
		gtk_window_set_title (GTK_WINDOW (window), PROGRAM_NAME " - Unsaved stock list");

		// Show status message
		ShowStatusMessage ("New stock list created", fileid);

		// Get stock list
		GtkTreeModel *model = GTK_TREE_MODEL (stocks.GetStockList ());

		// Set tree view model
		gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (model));

		// Set sort column
		SetSortColumn (GTK_TREE_VIEW (treeview), STOCK_TICKER_ID, GTK_SORT_ASCENDING);

		// Change document state
		ChangeDocumentState (NULL, tzone, TRUE);

		// Change save state
		saved = TRUE;

		// Change selection state
		ChangeSelectionState (FALSE);

		// Free temporary string buffer
		g_free (tzone);

		// Set success state
		status = TRUE;
	}

	// Destroy dialog widget
	gtk_widget_destroy (GTK_WIDGET (dialog));

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Open" menu button                                 //
//****************************************************************************//
static gboolean OpenStocks (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// If file is changed, then ask to save changes before continue
		if (!SaveChanges ())
		{
			// If can not save changes, then return fail status
			return FALSE;
		}
	}

	// Create dialog window
	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open stocks from...", GTK_WINDOW (window), GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

	// Create file filter
	GtkFileFilter *filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (GTK_FILE_FILTER (filter), "*.xml");

	// Add file filter to file chooser
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), GTK_FILE_FILTER (filter));

	// Set file chooser properties
	gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (dialog), FALSE);
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), TRUE);
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), FALSE);

	// Set default dialog button
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	// Run dialog window
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		// Get chosen file name
		gchar *path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		// Create error object
		GError *error = NULL;

		// Open stock list from file
		if (!stocks.OpenList (path, &error))
			ShowFileErrorMessage (GTK_WINDOW (dialog), "Can not open stock list", error);
		else
		{
			// Extract file name from path
			gchar *fname = g_filename_display_basename (path);

			// Get stock list time zone
			gchar *tzone = stocks.GetTimeZone ();

			// Create string buffers
			GString *tstring = g_string_new (NULL);
			GString *sstring = g_string_new (NULL);

			// Create title bar string
			g_string_printf (tstring, PROGRAM_NAME " - %s", fname);

			// Create status bar string
			g_string_printf (sstring, "Opened stock list file '%s'", fname);

			// Set main window title
			gtk_window_set_title (GTK_WINDOW (window), tstring -> str);

			// Show status message
			ShowStatusMessage (sstring -> str, fileid);

			// Free temporary string buffer
			g_free (fname);

			// Relase string buffers
			g_string_free (tstring, TRUE);
			g_string_free (sstring, TRUE);

			// Get stock list
			model = GTK_TREE_MODEL (stocks.GetStockList ());

			// Set tree view model
			gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (model));

			// Set sort column
			SetSortColumn (GTK_TREE_VIEW (treeview), STOCK_TICKER_ID, GTK_SORT_ASCENDING);

			// Change document state
			ChangeDocumentState (path, tzone, TRUE);

			// Change save state
			ChangeSaveState (TRUE);

			// Change selection state
			ChangeSelectionState (FALSE);

			// Free temporary string buffer
			g_free (tzone);

			// Set success state
			status = TRUE;
		}

		// Free temporary string buffer
		g_free (path);
	}

	// Destroy dialog widget
	gtk_widget_destroy (GTK_WIDGET (dialog));

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Save" menu button                                 //
//****************************************************************************//
static gboolean SaveStocks (void)
{
	// Check if file name is set
	if (file_name)
	{
		// Operation status
		gboolean status = FALSE;

		// Get tree model object from tree view
		GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
		if (model)
		{
			// Create error object
			GError *error = NULL;

			// Save stock list into file
			if (!stocks.SaveList (file_name, &error))
				ShowFileErrorMessage (GTK_WINDOW (window), "Can not save stock list", error);
			else
			{
				// Extract file name from path
				gchar *fname = g_filename_display_basename (file_name);

				// Create string buffer
				GString *sstring = g_string_new (NULL);

				// Create status bar string
				g_string_printf (sstring, "Stocks saved into file '%s'", fname);

				// Show status message
				ShowStatusMessage (sstring -> str, fileid);

				// Free temporary string buffer
				g_free (fname);

				// Relase string buffer
				g_string_free (sstring, TRUE);

				// Change save state
				ChangeSaveState (TRUE);

				// Set success state
				status = TRUE;
			}
		}

		// Return operation status
		return status;
	}
	else
		return SaveStocksAs ();
}

//****************************************************************************//
//      Signal handler for "SaveAs" menu button                               //
//****************************************************************************//
static gboolean SaveStocksAs (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Create dialog window
		GtkWidget *dialog = gtk_file_chooser_dialog_new ("Save stocks to...", GTK_WINDOW (window), GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);

		// Create file filter
		GtkFileFilter *filter = gtk_file_filter_new ();
		gtk_file_filter_add_pattern (GTK_FILE_FILTER (filter), "*.xml");

		// Add file filter to file chooser
		gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), GTK_FILE_FILTER (filter));

		// Set file chooser properties
		gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (dialog), FALSE);
		gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), TRUE);
		gtk_file_chooser_set_create_folders (GTK_FILE_CHOOSER (dialog), TRUE);
		gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "Stocks.xml");

		// Set default dialog button
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

		// Run dialog window
		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			// Get chosen file name
			gchar *path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

			// Create error object
			GError *error = NULL;

			// Save stock list into file
			if (!stocks.SaveList (path, &error))
				ShowFileErrorMessage (GTK_WINDOW (dialog), "Can not save stock list", error);
			else
			{
				// Extract file name from path
				gchar *fname = g_filename_display_basename (path);

				// Get stock list time zone
				gchar *tzone = stocks.GetTimeZone ();

				// Create string buffers
				GString *tstring = g_string_new (NULL);
				GString *sstring = g_string_new (NULL);

				// Create title bar string
				g_string_printf (tstring, PROGRAM_NAME " - %s", fname);

				// Create status bar string
				g_string_printf (sstring, "Stocks saved into file '%s'", fname);

				// Set main window title
				gtk_window_set_title (GTK_WINDOW (window), tstring -> str);

				// Show status message
				ShowStatusMessage (sstring -> str, fileid);

				// Free temporary string buffer
				g_free (fname);

				// Relase string buffers
				g_string_free (tstring, TRUE);
				g_string_free (sstring, TRUE);

				// Change document state
				ChangeDocumentState (path, tzone, TRUE);

				// Change save state
				ChangeSaveState (TRUE);

				// Free temporary string buffer
				g_free (tzone);

				// Set success state
				status = TRUE;
			}

			// Free temporary string buffer
			g_free (path);
		}

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Import" menu button                               //
//****************************************************************************//
static gboolean ImportStocks (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// If file is changed, then ask to save changes before continue
		if (!SaveChanges ())
		{
			// If can not save changes, then return fail status
			return FALSE;
		}

		// Create dialog window
		GtkWidget *dialog = gtk_file_chooser_dialog_new ("Import stocks from...", GTK_WINDOW (window), GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Import", GTK_RESPONSE_ACCEPT, NULL);

		// Create file filter
		GtkFileFilter *filter = gtk_file_filter_new ();
		gtk_file_filter_add_pattern (GTK_FILE_FILTER (filter), "*.tsv");

		// Add file filter to file chooser
		gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), GTK_FILE_FILTER (filter));

		// Set file chooser properties
		gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (dialog), FALSE);
		gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), TRUE);
		gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), FALSE);

		// Set default dialog button
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

		// Run dialog window
		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			// Get chosen file name
			gchar *path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

			// Create error object
			GError *error = NULL;

			// Import stock list from file
			if (!stocks.ImportList (path, &error))
				ShowFileErrorMessage (GTK_WINDOW (dialog), "Can not import stock list", error);
			else
			{
				// Extract file name from path
				gchar *fname = g_filename_display_basename (path);

				// Create string buffer
				GString *sstring = g_string_new (NULL);

				// Create status bar string
				g_string_printf (sstring, "Stocks imported from file '%s'", fname);

				// Show status message
				ShowStatusMessage (sstring -> str, fileid);

				// Free temporary string buffer
				g_free (fname);

				// Relase string buffer
				g_string_free (sstring, TRUE);

				// Get stock list
				model = GTK_TREE_MODEL (stocks.GetStockList ());

				// Set tree view model
				gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (model));

				// Set sort column
				SetSortColumn (GTK_TREE_VIEW (treeview), STOCK_TICKER_ID, GTK_SORT_ASCENDING);

				// Change save state
				ChangeSaveState (FALSE);

				// Set success state
				status = TRUE;
			}

			// Free temporary string buffer
			g_free (path);
		}

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Export" menu button                               //
//****************************************************************************//
static gboolean ExportStocks (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Create dialog window
		GtkWidget *dialog = gtk_file_chooser_dialog_new ("Export stocks to...", GTK_WINDOW (window), GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL, "_Export", GTK_RESPONSE_ACCEPT, NULL);

		// Create file filter
		GtkFileFilter *filter = gtk_file_filter_new ();
		gtk_file_filter_add_pattern (GTK_FILE_FILTER (filter), "*.tsv");

		// Add file filter to file chooser
		gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), GTK_FILE_FILTER (filter));

		// Set file chooser properties
		gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (dialog), FALSE);
		gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), TRUE);
		gtk_file_chooser_set_create_folders (GTK_FILE_CHOOSER (dialog), TRUE);
		gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "Stocks.tsv");

		// Set default dialog button
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

		// Run dialog window
		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			// Get chosen file name
			gchar *path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

			// Create error object
			GError *error = NULL;

			// Export stock list into file
			if (!stocks.ExportList (path, &error))
				ShowFileErrorMessage (GTK_WINDOW (dialog), "Can not export stock list", error);
			else
			{
				// Extract file name from path
				gchar *fname = g_filename_display_basename (path);

				// Create string buffer
				GString *sstring = g_string_new (NULL);

				// Create status bar string
				g_string_printf (sstring, "Stocks exported into file '%s'", fname);

				// Show status message
				ShowStatusMessage (sstring -> str, fileid);

				// Free temporary string buffer
				g_free (fname);

				// Relase string buffer
				g_string_free (sstring, TRUE);

				// Set success state
				status = TRUE;
			}

			// Free temporary string buffer
			g_free (path);
		}

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Properties" menu button                           //
//****************************************************************************//
static gboolean DocumentProperties (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Create dialog window
		GtkWidget *dialog = gtk_dialog_new_with_buttons ("Stock list properties", GTK_WINDOW (window), GTK_DIALOG_MODAL, "_Close", GTK_RESPONSE_CLOSE, NULL);

		// Get content area of dialog
		GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

		// Get action area of dialog
		GtkWidget *action = gtk_dialog_get_action_area (GTK_DIALOG (dialog));

		// Create alignment
		GtkWidget *alignment = gtk_alignment_new (0, 0, 1, 1);

		// Create grid
		GtkWidget *grid = gtk_grid_new ();

		// Create label fields
		GtkWidget *FileLabel = gtk_label_new (NULL);
		GtkWidget *ZoneLabel = gtk_label_new (NULL);
		GtkWidget *CountLabel = gtk_label_new (NULL);
		GtkWidget *ChangedLabel = gtk_label_new (NULL);
		GtkWidget *FileValue = gtk_label_new (NULL);
		GtkWidget *ZoneValue = gtk_label_new (NULL);
		GtkWidget *CountValue = gtk_label_new (NULL);
		GtkWidget *ChangedValue = gtk_label_new (NULL);

		// Add labels to grid
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (FileLabel), 0, 0, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (ZoneLabel), 0, 1, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (CountLabel), 0, 2, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (ChangedLabel), 0, 3, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (FileValue), 1, 0, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (ZoneValue), 1, 1, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (CountValue), 1, 2, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (ChangedValue), 1, 3, 1, 1);

		// Add grid to alignment
		gtk_container_add (GTK_CONTAINER (alignment), grid);

		// Add alignment to dialog content area
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (alignment), TRUE, TRUE, 0);

		// Set label properties
		gtk_label_set_selectable (GTK_LABEL (FileLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (ZoneLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (CountLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (ChangedLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (FileValue), TRUE);
		gtk_label_set_selectable (GTK_LABEL (ZoneValue), TRUE);
		gtk_label_set_selectable (GTK_LABEL (CountValue), TRUE);
		gtk_label_set_selectable (GTK_LABEL (ChangedValue), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (FileLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (ZoneLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (CountLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (ChangedLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (FileValue), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (ZoneValue), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (CountValue), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (ChangedValue), TRUE);
		gtk_widget_set_halign (GTK_WIDGET (FileLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (ZoneLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (CountLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (ChangedLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (FileValue), GTK_ALIGN_START);
		gtk_widget_set_halign (GTK_WIDGET (ZoneValue), GTK_ALIGN_START);
		gtk_widget_set_halign (GTK_WIDGET (CountValue), GTK_ALIGN_START);
		gtk_widget_set_halign (GTK_WIDGET (ChangedValue), GTK_ALIGN_START);

		// Set grid properties
		guint box_border = gtk_container_get_border_width (GTK_CONTAINER (box));
		guint action_border = gtk_container_get_border_width (GTK_CONTAINER (action));
		gtk_container_set_border_width (GTK_CONTAINER (grid), action_border);
		gtk_grid_set_row_spacing (GTK_GRID (grid), box_border);
		gtk_grid_set_column_spacing (GTK_GRID (grid), box_border + action_border);

		// Get stocks count
		gint count = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (model), NULL);

		// Create string buffer
		GString *cstring = g_string_new (NULL);

		// Correct stock list file name
		const gchar *fname;
		if (file_name)
			fname = file_name;
		else
			fname = "Not set";

		// Correct stock list changed state
		const gchar *changed;
		if (!saved)
			changed = "Yes";
		else
			changed = "No";

		// Convert stock count into string
		g_string_printf (cstring, "%i stocks", count);

		// Set stock list properties
		gtk_label_set_markup (GTK_LABEL (FileLabel), "<b>File name:</b>");
		gtk_label_set_markup (GTK_LABEL (ZoneLabel), "<b>Time zone:</b>");
		gtk_label_set_markup (GTK_LABEL (CountLabel), "<b>Stocks count:</b>");
		gtk_label_set_markup (GTK_LABEL (ChangedLabel), "<b>Changed:</b>");
		gtk_label_set_text (GTK_LABEL (FileValue), fname);
		gtk_label_set_text (GTK_LABEL (ZoneValue), time_zone);
		gtk_label_set_text (GTK_LABEL (CountValue), cstring -> str);
		gtk_label_set_text (GTK_LABEL (ChangedValue), changed);

		// Relase string buffer
		g_string_free (cstring, TRUE);

		// Show all box elements
		gtk_widget_show_all (GTK_WIDGET (box));

		// Run dialog window
		gtk_dialog_run (GTK_DIALOG (dialog));

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (dialog));

		// Set success state
		status = TRUE;
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Close" menu button                               //
//****************************************************************************//
static gboolean CloseDocument (void)
{
	// Operation status
	gboolean status = TRUE;

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// If file is changed, then ask to save changes before continue
		if (!SaveChanges ())
		{
			// If can not save changes, then return fail status
			return FALSE;
		}

		// Clear stock list
		stocks.ClearList ();

		// Set main window title
		gtk_window_set_title (GTK_WINDOW (window), PROGRAM_NAME);

		// Check stock file name
		if (file_name)
		{
			// Extract file name from path
			gchar *fname = g_filename_display_basename (file_name);

			// Create string buffer
			GString *sstring = g_string_new (NULL);

			// Create status bar string
			g_string_printf (sstring, "File '%s' closed", fname);

			// Show status message
			ShowStatusMessage (sstring -> str, fileid);

			// Free temporary string buffer
			g_free (fname);

			// Relase string buffer
			g_string_free (sstring, TRUE);
		}
		else
		{
			// Show status message
			ShowStatusMessage ("Stock list closed", fileid);
		}

		// Get stock list
		GtkTreeModel *model = GTK_TREE_MODEL (stocks.GetStockList ());

		// Set tree view model
		gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (model));

		// Change document state
		ChangeDocumentState (NULL, NULL, FALSE);
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Quit" menu button                                 //
//****************************************************************************//
static gboolean QuitProgram (void)
{
	// Try to close opened document before exit
	if (CloseDocument ())
	{
		// Quit program
		gtk_main_quit ();
	}

	// Return fail status
	return TRUE;
}

//****************************************************************************//
//      Signal handler for "Insert" menu button                               //
//****************************************************************************//
static gboolean InsertStock (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Create dialog window
		GtkWidget *dialog = gtk_dialog_new_with_buttons ("New stock details", GTK_WINDOW (window), GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL, "_Insert", GTK_RESPONSE_ACCEPT, NULL);

		// Get content area of dialog
		GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

		// Get action area of dialog
		GtkWidget *action = gtk_dialog_get_action_area (GTK_DIALOG (dialog));

		// Create alignment
		GtkWidget *alignment = gtk_alignment_new (0, 0, 1, 1);

		// Create grid
		GtkWidget *grid = gtk_grid_new ();

		// Create label fields
		GtkWidget *TickerLabel = gtk_label_new (STOCK_TICKER_LABEL);
		GtkWidget *NameLabel = gtk_label_new (STOCK_NAME_LABEL);
		GtkWidget *CountryLabel = gtk_label_new (STOCK_COUNTRY_LABEL);
		GtkWidget *SectorLabel = gtk_label_new (STOCK_SECTOR_LABEL);
		GtkWidget *IndustryLabel = gtk_label_new (STOCK_INDUSTRY_LABEL);
		GtkWidget *UrlLabel = gtk_label_new (STOCK_URL_LABEL);

		// Create text entry fields
		GtkWidget *TickerEntry = gtk_entry_new ();
		GtkWidget *NameEntry = gtk_entry_new ();
		GtkWidget *CountryEntry = gtk_entry_new ();
		GtkWidget *SectorEntry = gtk_entry_new ();
		GtkWidget *IndustryEntry = gtk_entry_new ();
		GtkWidget *UrlEntry = gtk_entry_new ();

		// Add labels to grid
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (TickerLabel), 0, 0, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (NameLabel), 0, 1, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (CountryLabel), 0, 2, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (SectorLabel), 0, 3, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (IndustryLabel), 0, 4, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (UrlLabel), 0, 5, 1, 1);

		// Add text entries to grid
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (TickerEntry), 1, 0, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (NameEntry), 1, 1, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (CountryEntry), 1, 2, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (SectorEntry), 1, 3, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (IndustryEntry), 1, 4, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (UrlEntry), 1, 5, 1, 1);

		// Add tooltip text to text entries
		gtk_widget_set_tooltip_text (GTK_WIDGET (TickerEntry), "Short stock ticker");
		gtk_widget_set_tooltip_text (GTK_WIDGET (NameEntry), "Full stock name");
		gtk_widget_set_tooltip_text (GTK_WIDGET (CountryEntry), "Stock country");
		gtk_widget_set_tooltip_text (GTK_WIDGET (SectorEntry), "Stock sector");
		gtk_widget_set_tooltip_text (GTK_WIDGET (IndustryEntry), "Stock industry");
		gtk_widget_set_tooltip_text (GTK_WIDGET (UrlEntry), "Url to stock detailed information");

		// Add grid to alignment
		gtk_container_add (GTK_CONTAINER (alignment), grid);

		// Add alignment to dialog content area
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (alignment), TRUE, TRUE, 0);

		// Set label properties
		gtk_label_set_selectable (GTK_LABEL (TickerLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (NameLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (CountryLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (SectorLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (IndustryLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (UrlLabel), FALSE);
		gtk_label_set_single_line_mode (GTK_LABEL (TickerLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (NameLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (CountryLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (SectorLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (IndustryLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (UrlLabel), TRUE);
		gtk_widget_set_halign (GTK_WIDGET (TickerLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (NameLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (CountryLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (SectorLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (IndustryLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (UrlLabel), GTK_ALIGN_END);

		// Set text entry properties
		gtk_entry_set_placeholder_text (GTK_ENTRY (CountryEntry), "Not obligatory");
		gtk_entry_set_placeholder_text (GTK_ENTRY (SectorEntry), "Not obligatory");
		gtk_entry_set_placeholder_text (GTK_ENTRY (IndustryEntry), "Not obligatory");
		gtk_entry_set_placeholder_text (GTK_ENTRY (UrlEntry), "http://");
		gtk_entry_set_width_chars (GTK_ENTRY (TickerEntry), ENTRY_MAX_CHARS);
		gtk_entry_set_width_chars (GTK_ENTRY (NameEntry), ENTRY_MAX_CHARS);
		gtk_entry_set_width_chars (GTK_ENTRY (CountryEntry), ENTRY_MAX_CHARS);
		gtk_entry_set_width_chars (GTK_ENTRY (SectorEntry), ENTRY_MAX_CHARS);
		gtk_entry_set_width_chars (GTK_ENTRY (IndustryEntry), ENTRY_MAX_CHARS);
		gtk_entry_set_width_chars (GTK_ENTRY (UrlEntry), ENTRY_MAX_CHARS);
		gtk_widget_set_hexpand (GTK_WIDGET (TickerEntry), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (NameEntry), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (CountryEntry), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (SectorEntry), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (IndustryEntry), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (UrlEntry), TRUE);

		// Set grid properties
		guint box_border = gtk_container_get_border_width (GTK_CONTAINER (box));
		guint action_border = gtk_container_get_border_width (GTK_CONTAINER (action));
		gtk_container_set_border_width (GTK_CONTAINER (grid), action_border);
		gtk_grid_set_column_spacing (GTK_GRID (grid), box_border + action_border);

		// Set default dialog button
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

		// Show all box elements
		gtk_widget_show_all (GTK_WIDGET (box));

		// Run dialog window
		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			// Extract stock information from dialog inputs
			gchar *ticker = g_strdup (gtk_entry_get_text (GTK_ENTRY (TickerEntry)));
			gchar *name = g_strdup (gtk_entry_get_text (GTK_ENTRY (NameEntry)));
			gchar *country = g_strdup (gtk_entry_get_text (GTK_ENTRY (CountryEntry)));
			gchar *sector = g_strdup (gtk_entry_get_text (GTK_ENTRY (SectorEntry)));
			gchar *industry = g_strdup (gtk_entry_get_text (GTK_ENTRY (IndustryEntry)));
			gchar *url = g_strdup (gtk_entry_get_text (GTK_ENTRY (UrlEntry)));

			// Strip stock fields
			ticker = g_strstrip (ticker);
			name = g_strstrip (name);
			country = g_strstrip (country);
			sector = g_strstrip (sector);
			industry = g_strstrip (industry);
			url = g_strstrip (url);

			// Create error object
			GError *error = NULL;

			// Convert ticker to upper case
			gchar *upper = g_utf8_strup (ticker, -1);

			// Check stock details
			if (!CheckField (ticker, "ticker", &error) || !CheckField (name, "name", &error) || !IsTickerUnique (GTK_TREE_MODEL (model), upper, &error))
				ShowErrorMessage (GTK_WINDOW (dialog), "Incorrect stock details", error);
			else
			{
				// Add new element to list store object
				GtkTreeIter iter;
				gtk_list_store_append (GTK_LIST_STORE (model), &iter);
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, STOCK_TICKER_ID, upper, STOCK_NAME_ID, name, STOCK_COUNTRY_ID, country, STOCK_SECTOR_ID, sector, STOCK_INDUSTRY_ID, industry, STOCK_URL_ID, url, STOCK_CHECK_ID, FALSE, -1);

				// Change save state
				ChangeSaveState (FALSE);

				// Set success state
				status = TRUE;
			}

			// Free temporary string buffers
			g_free (ticker);
			g_free (name);
			g_free (country);
			g_free (sector);
			g_free (industry);
			g_free (url);
			g_free (upper);
		}

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Remove" menu button                               //
//****************************************************************************//
static gboolean RemoveStocks (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
		{
			// Init loop processing flag
			gboolean flag = TRUE;

			// Init stock list changed flag
			gboolean changed = FALSE;

			// Iterate through all elements
			do {
				// Get mark state
				gboolean state;
				gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, STOCK_CHECK_ID, &state, -1);

				// Check mark state
				if (state)
				{
					// If stock is marked, then remove it
					flag = gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

					// Set stock list changed flag
					changed = TRUE;
				}
				else
				{
					// If stock is not marked then go to next stock
					flag = gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter);
				}
			} while (flag);

			// Check if stock list was changed
			if (changed)
			{
				// Change save state
				ChangeSaveState (FALSE);
			}
		}

		// Set success state
		status = TRUE;
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "SelectAll" menu button                            //
//****************************************************************************//
static gboolean SelectAllStocks (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeSortable *model = GTK_TREE_SORTABLE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));
	if (model)
	{
		// Get current sort order
		gint id;
		GtkSortType order;
		gboolean issorted = gtk_tree_sortable_get_sort_column_id (GTK_TREE_SORTABLE (model), &id, &order);

		// Change sort column
		gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), STOCK_URL_ID, GTK_SORT_ASCENDING);

		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
		{
			// Iterate through all elements
			do {
				// Set mark state
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, STOCK_CHECK_ID, TRUE, -1);

				// Change iterator position to next element
			} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

			// Set success state
			status = TRUE;
		}

		// If stocks were sorted, then restore their sort order
		if (issorted)
			gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), id, order);
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Invert" menu button                               //
//****************************************************************************//
static gboolean InvertSelection (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeSortable *model = GTK_TREE_SORTABLE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));
	if (model)
	{
		// Get current sort order
		gint id;
		GtkSortType order;
		gboolean issorted = gtk_tree_sortable_get_sort_column_id (GTK_TREE_SORTABLE (model), &id, &order);

		// Change sort column
		gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), STOCK_URL_ID, GTK_SORT_ASCENDING);

		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
		{
			// Iterate through all elements
			do {
				// Get old state
				gboolean state;
				gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, STOCK_CHECK_ID, &state, -1);

				// Switch mark state
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, STOCK_CHECK_ID, !state, -1);

				// Change iterator position to next element
			} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

			// Set success state
			status = TRUE;
		}

		// If stocks were sorted, then restore their sort order
		if (issorted)
			gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), id, order);
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "UnselectAll" menu button                          //
//****************************************************************************//
gboolean UnselectAllStocks (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeSortable *model = GTK_TREE_SORTABLE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));
	if (model)
	{
		// Get current sort order
		gint id;
		GtkSortType order;
		gboolean issorted = gtk_tree_sortable_get_sort_column_id (GTK_TREE_SORTABLE (model), &id, &order);

		// Change sort column
		gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), STOCK_URL_ID, GTK_SORT_ASCENDING);

		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
		{
			// Iterate through all elements
			do {
				// Clear mark state
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, STOCK_CHECK_ID, FALSE, -1);

				// Change iterator position to next element
			} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

			// Set success state
			status = TRUE;
		}

		// If stocks were sorted, then restore their sort order
		if (issorted)
			gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), id, order);
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Find" menu button                                 //
//****************************************************************************//
static gboolean FindStocks (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeSortable *model = GTK_TREE_SORTABLE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));
	if (model)
	{
		// Create dialog window
		GtkWidget *dialog = gtk_dialog_new_with_buttons ("Find", GTK_WINDOW (window), GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL, "_Find", GTK_RESPONSE_ACCEPT, NULL);

		// Get content area of dialog
		GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

		// Get action area of dialog
		GtkWidget *action = gtk_dialog_get_action_area (GTK_DIALOG (dialog));

		// Create alignment
		GtkWidget *alignment = gtk_alignment_new (0, 0, 1, 1);

		// Create grid
		GtkWidget *grid = gtk_grid_new ();

		// Create label fields
		GtkWidget *FindLabel = gtk_label_new ("Search by");
		GtkWidget *PatternLabel = gtk_label_new ("Pattern");

		// Create combo box
		GtkWidget *FindBox = gtk_combo_box_text_new ();

		// Create text entry field
		GtkWidget *PatternEntry = gtk_entry_new ();

		// Add options to combo box
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (FindBox), STOCK_TICKER_LABEL);
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (FindBox), STOCK_NAME_LABEL);
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (FindBox), STOCK_COUNTRY_LABEL);
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (FindBox), STOCK_SECTOR_LABEL);
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (FindBox), STOCK_INDUSTRY_LABEL);

		// Add labels to grid
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (FindLabel), 0, 0, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (PatternLabel), 0, 1, 1, 1);

		// Add combo box to grid
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (FindBox), 1, 0, 1, 1);

		// Add text entry to grid
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (PatternEntry), 1, 1, 1, 1);

		// Add tooltip text to combo box and text entry
		gtk_widget_set_tooltip_text (GTK_WIDGET (FindBox), "Stock field to search");
		gtk_widget_set_tooltip_text (GTK_WIDGET (PatternEntry), "Glob-style pattern to find");

		// Add grid to alignment
		gtk_container_add (GTK_CONTAINER (alignment), grid);

		// Add alignment to dialog content area
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (alignment), TRUE, TRUE, 0);

		// Set label properties
		gtk_label_set_selectable (GTK_LABEL (FindLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (PatternLabel), FALSE);
		gtk_label_set_single_line_mode (GTK_LABEL (FindLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (PatternLabel), TRUE);
		gtk_widget_set_halign (GTK_WIDGET (FindLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (PatternLabel), GTK_ALIGN_END);

		// Set combo box properties
		gtk_combo_box_set_button_sensitivity (GTK_COMBO_BOX (FindBox), GTK_SENSITIVITY_AUTO);
		gtk_combo_box_set_active (GTK_COMBO_BOX (FindBox), STOCK_TICKER_ID);
		gtk_combo_box_set_popup_fixed_width (GTK_COMBO_BOX (FindBox), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (FindBox), TRUE);

		// Set text entry properties
		gtk_entry_set_placeholder_text (GTK_ENTRY (PatternEntry), "Glob-style pattern");
		gtk_entry_set_activates_default (GTK_ENTRY (PatternEntry), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (PatternEntry), TRUE);

		// Set grid properties
		guint box_border = gtk_container_get_border_width (GTK_CONTAINER (box));
		guint action_border = gtk_container_get_border_width (GTK_CONTAINER (action));
		gtk_container_set_border_width (GTK_CONTAINER (grid), action_border);
		gtk_grid_set_column_spacing (GTK_GRID (grid), box_border + action_border);

		// Set default dialog button
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

		// Show all box elements
		gtk_widget_show_all (GTK_WIDGET (box));

		// Run dialog window
		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			// Get search field id
			gint id = gtk_combo_box_get_active (GTK_COMBO_BOX (FindBox));
			if (id != -1)
			{
				// Get pattern value
				const gchar *pattern = gtk_entry_get_text (GTK_ENTRY (PatternEntry));

				// Check if pattern value is not empty
				if (g_utf8_strlen (pattern, -1))
				{
					// Convert pattern value to case independent representation
					gchar *ipattern = g_utf8_casefold (pattern, -1);

					// Create glob-style pattern object
					GPatternSpec *gpattern = g_pattern_spec_new (ipattern);

					// Change sort column
					gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), STOCK_URL_ID, GTK_SORT_ASCENDING);

					// Get iterator position
					GtkTreeIter iter;
					if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
					{
						// Iterate through all elements
						do {
							// Get required stock field
							gchar *value;
							gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, id, &value, -1);

							// Convert field value to case independent representation
							gchar *ivalue = g_utf8_casefold (value, -1);

							// Check if value has required substring pattern
							if (g_pattern_match_string (gpattern, ivalue))
							{
								// Set mark state
								gtk_list_store_set (GTK_LIST_STORE (model), &iter, STOCK_CHECK_ID, TRUE, -1);
							}
							else
							{
								// Clear mark state
								gtk_list_store_set (GTK_LIST_STORE (model), &iter, STOCK_CHECK_ID, FALSE, -1);
							}

							// Free temporary string buffers
							g_free (value);
							g_free (ivalue);

							// Change iterator position to next element
						} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

						// Set search column for immediate search
						gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), id);

						// Move found stocks on top of stock list
						gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), STOCK_CHECK_ID, GTK_SORT_DESCENDING);

						// Set success state
						status = TRUE;
					}

					// Free temporary string buffer
					g_free (ipattern);

					// Free glob-style pattern object
					g_pattern_spec_free (gpattern);
				}
			}
		}

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Sort" menu button                                 //
//****************************************************************************//
static gboolean SortStocks (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeSortable *model = GTK_TREE_SORTABLE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));
	if (model)
	{
		// Create dialog window
		GtkWidget *dialog = gtk_dialog_new_with_buttons ("Sort properties", GTK_WINDOW (window), GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL, "_Sort", GTK_RESPONSE_ACCEPT, NULL);

		// Get content area of dialog
		GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

		// Get action area of dialog
		GtkWidget *action = gtk_dialog_get_action_area (GTK_DIALOG (dialog));

		// Create alignment
		GtkWidget *alignment = gtk_alignment_new (0, 0, 1, 1);

		// Create grid
		GtkWidget *grid = gtk_grid_new ();

		// Create label fields
		GtkWidget *SortLabel = gtk_label_new ("Sort stocks by");

		// Create combo box
		GtkWidget *SortBox = gtk_combo_box_text_new ();

		// Create radio buttons
		GtkWidget *AscOrder = gtk_radio_button_new_with_label (NULL, "Ascending order");
		GtkWidget *DscOrder = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (AscOrder), "Descending order");

		// Add options to combo box
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (SortBox), STOCK_TICKER_LABEL);
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (SortBox), STOCK_NAME_LABEL);
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (SortBox), STOCK_COUNTRY_LABEL);
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (SortBox), STOCK_SECTOR_LABEL);
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (SortBox), STOCK_INDUSTRY_LABEL);

		// Add label to grid
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (SortLabel), 0, 0, 1, 1);

		// Add combo box to grid
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (SortBox), 1, 0, 1, 1);

		// Add radio buttons to grid
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (AscOrder), 0, 1, 2, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (DscOrder), 0, 2, 2, 1);

		// Add tooltip text to combo box and radio buttons
		gtk_widget_set_tooltip_text (GTK_WIDGET (SortBox), "Stock field to sort by");
		gtk_widget_set_tooltip_text (GTK_WIDGET (AscOrder), "Sort stocks in ascending order");
		gtk_widget_set_tooltip_text (GTK_WIDGET (DscOrder), "Sort stocks in descending order");

		// Add grid to alignment
		gtk_container_add (GTK_CONTAINER (alignment), grid);

		// Add alignment to dialog content area
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (alignment), TRUE, TRUE, 0);

		// Set label properties
		gtk_label_set_selectable (GTK_LABEL (SortLabel), FALSE);
		gtk_label_set_single_line_mode (GTK_LABEL (SortLabel), TRUE);
		gtk_widget_set_halign (GTK_WIDGET (SortLabel), GTK_ALIGN_END);

		// Set combo box properties
		gtk_combo_box_set_button_sensitivity (GTK_COMBO_BOX (SortBox), GTK_SENSITIVITY_AUTO);
		gtk_combo_box_set_active (GTK_COMBO_BOX (SortBox), STOCK_TICKER_ID);
		gtk_combo_box_set_popup_fixed_width (GTK_COMBO_BOX (SortBox), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (SortBox), TRUE);

		// Set grid properties
		guint box_border = gtk_container_get_border_width (GTK_CONTAINER (box));
		guint action_border = gtk_container_get_border_width (GTK_CONTAINER (action));
		gtk_container_set_border_width (GTK_CONTAINER (grid), action_border);
		gtk_grid_set_column_spacing (GTK_GRID (grid), box_border + action_border);

		// Set default dialog button
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

		// Show all box elements
		gtk_widget_show_all (GTK_WIDGET (box));

		// Run dialog window
		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			// Get sort field id
			gint id = gtk_combo_box_get_active (GTK_COMBO_BOX (SortBox));
			if (id != -1)
			{
				// Get sort order
				GtkSortType order = GTK_SORT_ASCENDING;
				if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (DscOrder)))
					order = GTK_SORT_DESCENDING;

				// Set search column for immediate search
				gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), id);

				// Sort stocks by chosen field
				gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), id, order);

				// Set success state
				status = TRUE;
			}
		}

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Info" menu button                                 //
//****************************************************************************//
static gboolean StockInfo (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree view selection
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

	// Check if any stock is selected
	GtkTreeModel *model;
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), &model, &iter))
	{
		// Check if stock list is saved
		if (file_name == NULL)
			AskToSaveStockList ();
		else
		{
			// Get stock details
			gchar *ticker;
			gchar *name;
			gchar *country;
			gchar *sector;
			gchar *industry;
			gchar *url;
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, STOCK_TICKER_ID, &ticker, STOCK_NAME_ID, &name, STOCK_COUNTRY_ID, &country, STOCK_SECTOR_ID, &sector, STOCK_INDUSTRY_ID, &industry, STOCK_URL_ID, &url, -1);

			// Get quotes file name
			gchar* path = GetQuotesFile (file_name, ticker);

			// Create dialog window
			GtkWidget *dialog = gtk_dialog_new_with_buttons ("Stock information", GTK_WINDOW (window), GTK_DIALOG_DESTROY_WITH_PARENT, "_Close", GTK_RESPONSE_CLOSE, NULL);

			// Get content area of dialog
			GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

			// Get action area of dialog
			GtkWidget *action = gtk_dialog_get_action_area (GTK_DIALOG (dialog));

			// Create alignment
			GtkWidget *alignment = gtk_alignment_new (0, 0, 1, 1);

			// Create grid
			GtkWidget *grid = gtk_grid_new ();

			// Create label fields
			GtkWidget *TickerLabel = gtk_label_new (NULL);
			GtkWidget *NameLabel = gtk_label_new (NULL);
			GtkWidget *CountryLabel = gtk_label_new (NULL);
			GtkWidget *SectorLabel = gtk_label_new (NULL);
			GtkWidget *IndustryLabel = gtk_label_new (NULL);
			GtkWidget *QuotesLabel = gtk_label_new (NULL);
			GtkWidget *FirstLabel = gtk_label_new (NULL);
			GtkWidget *LastLabel = gtk_label_new (NULL);
			GtkWidget *SyncLabel = gtk_label_new (NULL);
			GtkWidget *PriceLabel = gtk_label_new (NULL);
			GtkWidget *FileLabel = gtk_label_new (NULL);
			GtkWidget *UrlLabel = gtk_label_new (NULL);
			GtkWidget *TickerValue = gtk_label_new (NULL);
			GtkWidget *NameValue = gtk_label_new (NULL);
			GtkWidget *CountryValue = gtk_label_new (NULL);
			GtkWidget *SectorValue = gtk_label_new (NULL);
			GtkWidget *IndustryValue = gtk_label_new (NULL);
			GtkWidget *QuotesValue = gtk_label_new (NULL);
			GtkWidget *FirstValue = gtk_label_new (NULL);
			GtkWidget *LastValue = gtk_label_new (NULL);
			GtkWidget *SyncValue = gtk_label_new (NULL);
			GtkWidget *PriceValue = gtk_label_new (NULL);
			GtkWidget *FileValue = gtk_label_new (NULL);
			GtkWidget *UrlValue = gtk_label_new (NULL);

			// Add labels to grid
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (TickerLabel), 0, 0, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (NameLabel), 0, 1, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (CountryLabel), 0, 2, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (SectorLabel), 0, 3, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (IndustryLabel), 0, 4, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (QuotesLabel), 0, 5, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (FirstLabel), 0, 6, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (LastLabel), 0, 7, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (SyncLabel), 0, 8, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (PriceLabel), 0, 9, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (FileLabel), 0, 10, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (UrlLabel), 0, 11, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (TickerValue), 1, 0, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (NameValue), 1, 1, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (CountryValue), 1, 2, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (SectorValue), 1, 3, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (IndustryValue), 1, 4, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (QuotesValue), 1, 5, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (FirstValue), 1, 6, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (LastValue), 1, 7, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (SyncValue), 1, 8, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (PriceValue), 1, 9, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (FileValue), 1, 10, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (UrlValue), 1, 11, 1, 1);

			// Add grid to alignment
			gtk_container_add (GTK_CONTAINER (alignment), grid);

			// Add alignment to dialog content area
			gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (alignment), TRUE, TRUE, 0);

			// Set label properties
			gtk_label_set_selectable (GTK_LABEL (TickerLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (NameLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (CountryLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (SectorLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (IndustryLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (QuotesLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (FirstLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (LastLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (SyncLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (PriceLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (FileLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (UrlLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (TickerValue), TRUE);
			gtk_label_set_selectable (GTK_LABEL (NameValue), TRUE);
			gtk_label_set_selectable (GTK_LABEL (CountryValue), TRUE);
			gtk_label_set_selectable (GTK_LABEL (SectorValue), TRUE);
			gtk_label_set_selectable (GTK_LABEL (IndustryValue), TRUE);
			gtk_label_set_selectable (GTK_LABEL (QuotesValue), TRUE);
			gtk_label_set_selectable (GTK_LABEL (FirstValue), TRUE);
			gtk_label_set_selectable (GTK_LABEL (LastValue), TRUE);
			gtk_label_set_selectable (GTK_LABEL (SyncValue), TRUE);
			gtk_label_set_selectable (GTK_LABEL (PriceValue), TRUE);
			gtk_label_set_selectable (GTK_LABEL (FileValue), TRUE);
			gtk_label_set_selectable (GTK_LABEL (UrlValue), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (TickerLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (NameLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (CountryLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (SectorLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (IndustryLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (QuotesLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (FirstLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (LastLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (SyncLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (PriceLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (FileLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (UrlLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (TickerValue), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (NameValue), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (CountryValue), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (SectorValue), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (IndustryValue), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (QuotesValue), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (FirstValue), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (LastValue), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (SyncValue), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (PriceValue), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (FileValue), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (UrlValue), TRUE);
			gtk_widget_set_halign (GTK_WIDGET (TickerLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (NameLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (CountryLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (SectorLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (IndustryLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (QuotesLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (FirstLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (LastLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (SyncLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (PriceLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (FileLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (UrlLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (TickerValue), GTK_ALIGN_START);
			gtk_widget_set_halign (GTK_WIDGET (NameValue), GTK_ALIGN_START);
			gtk_widget_set_halign (GTK_WIDGET (CountryValue), GTK_ALIGN_START);
			gtk_widget_set_halign (GTK_WIDGET (SectorValue), GTK_ALIGN_START);
			gtk_widget_set_halign (GTK_WIDGET (IndustryValue), GTK_ALIGN_START);
			gtk_widget_set_halign (GTK_WIDGET (QuotesValue), GTK_ALIGN_START);
			gtk_widget_set_halign (GTK_WIDGET (FirstValue), GTK_ALIGN_START);
			gtk_widget_set_halign (GTK_WIDGET (LastValue), GTK_ALIGN_START);
			gtk_widget_set_halign (GTK_WIDGET (SyncValue), GTK_ALIGN_START);
			gtk_widget_set_halign (GTK_WIDGET (PriceValue), GTK_ALIGN_START);
			gtk_widget_set_halign (GTK_WIDGET (FileValue), GTK_ALIGN_START);
			gtk_widget_set_halign (GTK_WIDGET (UrlValue), GTK_ALIGN_START);

			// Set grid properties
			guint box_border = gtk_container_get_border_width (GTK_CONTAINER (box));
			guint action_border = gtk_container_get_border_width (GTK_CONTAINER (action));
			gtk_container_set_border_width (GTK_CONTAINER (grid), action_border);
			gtk_grid_set_row_spacing (GTK_GRID (grid), box_border);
			gtk_grid_set_column_spacing (GTK_GRID (grid), box_border + action_border);

			// Set quote details to default values
			gint count = -1;
			time_t first = TIME_ERROR;
			time_t last = TIME_ERROR;
			time_t sync = TIME_ERROR;
			gfloat price = 0;

			// Create quotes object
			Quotes quotes;

			// Create error object
			GError *error = NULL;

			// Try to open quotes file
			if (!quotes.OpenList (path, &error))
			{
				// Show status message
				ShowStatusMessage (error -> message, fileid);

				// Release error object
				g_error_free (error);

				// Make unset fields insensitive
				gtk_widget_set_sensitive (GTK_WIDGET (QuotesValue), FALSE);
				gtk_widget_set_sensitive (GTK_WIDGET (FirstValue), FALSE);
				gtk_widget_set_sensitive (GTK_WIDGET (LastValue), FALSE);
				gtk_widget_set_sensitive (GTK_WIDGET (SyncValue), FALSE);
				gtk_widget_set_sensitive (GTK_WIDGET (PriceValue), FALSE);
			}
			else
			{
				// Get quote details
				count = quotes.GetCount ();
				first = quotes.GetFirstDate ();
				last = quotes.GetLastDate ();
				sync = quotes.GetSyncTime ();
				price = quotes.GetLastPrice ();
			}

			// Create string buffers
			GString *cstring = g_string_new (NULL);
			GString *fstring = g_string_new (NULL);
			GString *lstring = g_string_new (NULL);
			GString *sstring = g_string_new (NULL);
			GString *pstring = g_string_new (NULL);
			GString *ustring = g_string_new (NULL);

			// Correct country value
			if (g_utf8_strlen (country, -1) == 0)
			{
				g_free (country);
				country = g_strdup (STRING_UNKNOWN);
			}

			// Correct sector value
			if (g_utf8_strlen (sector, -1) == 0)
			{
				g_free (sector);
				sector = g_strdup (STRING_UNKNOWN);
			}

			// Correct industry value
			if (g_utf8_strlen (industry, -1) == 0)
			{
				g_free (industry);
				industry = g_strdup (STRING_UNKNOWN);
			}

			// Correct quotes count
			if (count == -1)
				g_string_printf (cstring, STRING_UNKNOWN);
			else
				g_string_printf (cstring, "%i quotes", count);

			// Correct first quote date
			if (first == static_cast <time_t> (TIME_ERROR))
				g_string_printf (fstring, STRING_UNKNOWN);
			else
			{
				date_struct date = Time::ExtractDate (first);
				g_string_printf (fstring, "%.4i-%.2d-%.2d", date.year, date.mon, date.day);
			}

			// Correct last quote date
			if (last == static_cast <time_t> (TIME_ERROR))
				g_string_printf (lstring, STRING_UNKNOWN);
			else
			{
				date_struct date = Time::ExtractDate (last);
				g_string_printf (lstring, "%.4i-%.2d-%.2d", date.year, date.mon, date.day);
			}

			// Correct quotes sync time
			if (sync == static_cast <time_t> (TIME_ERROR))
				g_string_printf (sstring, STRING_UNKNOWN);
			else
			{
				date_struct date = Time::ExtractDate (sync);
				g_string_printf (sstring, "%.4i-%.2d-%.2d", date.year, date.mon, date.day);
			}

			// Correct last quote price
			if (price == 0)
				g_string_printf (pstring, STRING_UNKNOWN);
			else
				g_string_printf (pstring, "$%.2f", price);

			// Correct stock url
			if (g_utf8_strlen (url, -1) == 0)
				g_string_printf (ustring, "No stock details");
			else
				g_string_printf (ustring, "<a href=\"%s\">%s</a>", url, url);

			// Set stock and quote details
			gtk_label_set_markup (GTK_LABEL (TickerLabel), "<b>" STOCK_TICKER_LABEL ":</b>");
			gtk_label_set_markup (GTK_LABEL (NameLabel), "<b>" STOCK_NAME_LABEL ":</b>");
			gtk_label_set_markup (GTK_LABEL (CountryLabel), "<b>" STOCK_COUNTRY_LABEL ":</b>");
			gtk_label_set_markup (GTK_LABEL (SectorLabel), "<b>" STOCK_SECTOR_LABEL ":</b>");
			gtk_label_set_markup (GTK_LABEL (IndustryLabel), "<b>" STOCK_INDUSTRY_LABEL ":</b>");
			gtk_label_set_markup (GTK_LABEL (QuotesLabel), "<b>Quotes:</b>");
			gtk_label_set_markup (GTK_LABEL (FirstLabel), "<b>First quote:</b>");
			gtk_label_set_markup (GTK_LABEL (LastLabel), "<b>Last quote:</b>");
			gtk_label_set_markup (GTK_LABEL (SyncLabel), "<b>Sync time:</b>");
			gtk_label_set_markup (GTK_LABEL (PriceLabel), "<b>Last price:</b>");
			gtk_label_set_markup (GTK_LABEL (FileLabel), "<b>Quotes file:</b>");
			gtk_label_set_markup (GTK_LABEL (UrlLabel), "<b>Details:</b>");
			gtk_label_set_text (GTK_LABEL (TickerValue), ticker);
			gtk_label_set_text (GTK_LABEL (NameValue), name);
			gtk_label_set_text (GTK_LABEL (CountryValue), country);
			gtk_label_set_text (GTK_LABEL (SectorValue), sector);
			gtk_label_set_text (GTK_LABEL (IndustryValue), industry);
			gtk_label_set_text (GTK_LABEL (QuotesValue), cstring -> str);
			gtk_label_set_text (GTK_LABEL (FirstValue), fstring -> str);
			gtk_label_set_text (GTK_LABEL (LastValue), lstring -> str);
			gtk_label_set_text (GTK_LABEL (SyncValue), sstring -> str);
			gtk_label_set_text (GTK_LABEL (PriceValue), pstring -> str);
			gtk_label_set_text (GTK_LABEL (FileValue), path);
			gtk_label_set_markup (GTK_LABEL (UrlValue), ustring -> str);

			// Free temporary string buffers
			g_free (ticker);
			g_free (name);
			g_free (country);
			g_free (sector);
			g_free (industry);
			g_free (url);
			g_free (path);

			// Relase string buffers
			g_string_free (cstring, TRUE);
			g_string_free (fstring, TRUE);
			g_string_free (lstring, TRUE);
			g_string_free (sstring, TRUE);
			g_string_free (pstring, TRUE);
			g_string_free (ustring, TRUE);

			// Show all box elements
			gtk_widget_show_all (GTK_WIDGET (box));

			// Run dialog window
			gtk_dialog_run (GTK_DIALOG (dialog));

			// Destroy dialog widget
			gtk_widget_destroy (GTK_WIDGET (dialog));

			// Set success state
			status = TRUE;
		}
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Check" menu button                                //
//****************************************************************************//
static gboolean CheckStocks (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Check if stock list is saved
		if (file_name == NULL)
			AskToSaveStockList ();
		else
		{
			// Run check quotes dialog
			status = CheckQuotesDialog (GTK_WINDOW (window), GTK_TREE_MODEL (model), file_name);
		}
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Sync" menu button                                 //
//****************************************************************************//
static gboolean SyncStocks (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Check if stock list is saved
		if (file_name == NULL)
			AskToSaveStockList ();
		else
		{
			// Run sync quotes dialog
			status = SyncQuotesDialog (GTK_WINDOW (window), GTK_TREE_MODEL (model), file_name, time_zone);
		}
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Analyze" menu button                              //
//****************************************************************************//
static gboolean AnalyzeStocks (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Check if stock list is saved
		if (file_name == NULL)
			AskToSaveStockList ();
		else
		{
			// Create dialog window
			GtkWidget *dialog = gtk_dialog_new_with_buttons ("Stock filter settings", GTK_WINDOW (window), GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL, "_Analyze", GTK_RESPONSE_ACCEPT, NULL);

			// Get content area of dialog
			GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

			// Get action area of dialog
			GtkWidget *action = gtk_dialog_get_action_area (GTK_DIALOG (dialog));

			// Create alignment
			GtkWidget *alignment = gtk_alignment_new (0, 0, 1, 1);

			// Create grid
			GtkWidget *grid = gtk_grid_new ();

			// Create label fields
			GtkWidget *QuotesLabel = gtk_label_new ("Min quotes");
			GtkWidget *LiquidityLabel = gtk_label_new ("Min liquidity");
			GtkWidget *VolatilityLabel = gtk_label_new ("Min volatility");
			GtkWidget *PriceLabel = gtk_label_new ("Min price");

			// Create spin buttons
			GtkWidget *QuotesSpin = gtk_spin_button_new_with_range (QUOTES_MIN, QUOTES_MAX, QUOTES_STEP);
			GtkWidget *LiquiditySpin = gtk_spin_button_new_with_range (LIQUIDITY_MIN, LIQUIDITY_MAX, LIQUIDITY_STEP);
			GtkWidget *VolatilitySpin = gtk_spin_button_new_with_range (VOLATILITY_MIN, VOLATILITY_MAX, VOLATILITY_STEP);
			GtkWidget *PriceSpin = gtk_spin_button_new_with_range (PRICE_MIN, PRICE_MAX, PRICE_STEP);

			// Add labels to grid
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (QuotesLabel), 0, 0, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (LiquidityLabel), 0, 1, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (VolatilityLabel), 0, 2, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (PriceLabel), 0, 3, 1, 1);

			// Add spin buttons to grid
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (QuotesSpin), 1, 0, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (LiquiditySpin), 1, 1, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (VolatilitySpin), 1, 2, 1, 1);
			gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (PriceSpin), 1, 3, 1, 1);

			// Add tooltip text to spin buttons
			gtk_widget_set_tooltip_text (GTK_WIDGET (QuotesSpin), "Min count of quotes to analyze");
			gtk_widget_set_tooltip_text (GTK_WIDGET (LiquiditySpin), "Min stock liquidity");
			gtk_widget_set_tooltip_text (GTK_WIDGET (VolatilitySpin), "Min stock volatility");
			gtk_widget_set_tooltip_text (GTK_WIDGET (PriceSpin), "Min stock price");

			// Add grid to alignment
			gtk_container_add (GTK_CONTAINER (alignment), grid);

			// Add alignment to dialog content area
			gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (alignment), TRUE, TRUE, 0);

			// Set label properties
			gtk_label_set_selectable (GTK_LABEL (QuotesLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (LiquidityLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (VolatilityLabel), FALSE);
			gtk_label_set_selectable (GTK_LABEL (PriceLabel), FALSE);
			gtk_label_set_single_line_mode (GTK_LABEL (QuotesLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (LiquidityLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (VolatilityLabel), TRUE);
			gtk_label_set_single_line_mode (GTK_LABEL (PriceLabel), TRUE);
			gtk_widget_set_halign (GTK_WIDGET (QuotesLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (LiquidityLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (VolatilityLabel), GTK_ALIGN_END);
			gtk_widget_set_halign (GTK_WIDGET (PriceLabel), GTK_ALIGN_END);

			// Set spin button properties
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (QuotesSpin), QUOTES_DEFAULT);
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (LiquiditySpin), LIQUIDITY_DEFAULT);
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (VolatilitySpin), VOLATILITY_DEFAULT);
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (PriceSpin), PRICE_DEFAULT);
			gtk_spin_button_set_digits (GTK_SPIN_BUTTON (QuotesSpin), QUOTES_DIGITS);
			gtk_spin_button_set_digits (GTK_SPIN_BUTTON (LiquiditySpin), LIQUIDITY_DIGITS);
			gtk_spin_button_set_digits (GTK_SPIN_BUTTON (VolatilitySpin), VOLATILITY_DIGITS);
			gtk_spin_button_set_digits (GTK_SPIN_BUTTON (PriceSpin), PRICE_DIGITS);
			gtk_spin_button_set_increments (GTK_SPIN_BUTTON (QuotesSpin), QUOTES_STEP, QUOTES_PAGE);
			gtk_spin_button_set_increments (GTK_SPIN_BUTTON (LiquiditySpin), LIQUIDITY_STEP, LIQUIDITY_PAGE);
			gtk_spin_button_set_increments (GTK_SPIN_BUTTON (VolatilitySpin), VOLATILITY_STEP, VOLATILITY_PAGE);
			gtk_spin_button_set_increments (GTK_SPIN_BUTTON (PriceSpin), PRICE_STEP, PRICE_PAGE);
			gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (QuotesSpin), GTK_UPDATE_IF_VALID);
			gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (LiquiditySpin), GTK_UPDATE_IF_VALID);
			gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (VolatilitySpin), GTK_UPDATE_IF_VALID);
			gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (PriceSpin), GTK_UPDATE_IF_VALID);
			gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (QuotesSpin), TRUE);
			gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (LiquiditySpin), TRUE);
			gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (VolatilitySpin), TRUE);
			gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (PriceSpin), TRUE);
			gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (QuotesSpin), TRUE);
			gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (LiquiditySpin), TRUE);
			gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (VolatilitySpin), TRUE);
			gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (PriceSpin), TRUE);
			gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (QuotesSpin), FALSE);
			gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (LiquiditySpin), FALSE);
			gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (VolatilitySpin), FALSE);
			gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (PriceSpin), FALSE);
			gtk_widget_set_hexpand (GTK_WIDGET (QuotesSpin), TRUE);
			gtk_widget_set_hexpand (GTK_WIDGET (LiquiditySpin), TRUE);
			gtk_widget_set_hexpand (GTK_WIDGET (VolatilitySpin), TRUE);
			gtk_widget_set_hexpand (GTK_WIDGET (PriceSpin), TRUE);

			// Set grid properties
			guint box_border = gtk_container_get_border_width (GTK_CONTAINER (box));
			guint action_border = gtk_container_get_border_width (GTK_CONTAINER (action));
			gtk_container_set_border_width (GTK_CONTAINER (grid), action_border);
			gtk_grid_set_column_spacing (GTK_GRID (grid), box_border + action_border);

			// Set default dialog button
			gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

			// Show all box elements
			gtk_widget_show_all (GTK_WIDGET (box));

			// Run dialog window
			gint response = gtk_dialog_run (GTK_DIALOG (dialog));

			// Extract stock filter settings from dialog inputs
			gint quotes = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (QuotesSpin));
			gsize liquidity = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (LiquiditySpin));
			gfloat volatility = gtk_spin_button_get_value (GTK_SPIN_BUTTON (VolatilitySpin));
			gfloat price = gtk_spin_button_get_value (GTK_SPIN_BUTTON (PriceSpin));

			// Destroy dialog widget
			gtk_widget_destroy (GTK_WIDGET (dialog));

			// Check if user chose to analyze stock list
			if (response == GTK_RESPONSE_ACCEPT)
			{
				// Run analyze quotes dialog
				status = AnalyzeQuotesDialog (GTK_WINDOW (window), GTK_TREE_MODEL (model), file_name, quotes, liquidity, volatility * 0.01, price);
			}
		}
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "View" menu button                                 //
//****************************************************************************//
static gboolean ViewQuotes (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree view selection
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

	// Check if any stock is selected
	GtkTreeModel *model;
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), &model, &iter))
	{
		// Check if stock list is saved
		if (file_name == NULL)
			AskToSaveStockList ();
		else
		{
			// Get stock details
			gchar *ticker;
			gchar *name;
			gchar *country;
			gchar *sector;
			gchar *industry;
			gchar *url;
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, STOCK_TICKER_ID, &ticker, STOCK_NAME_ID, &name, STOCK_COUNTRY_ID, &country, STOCK_SECTOR_ID, &sector, STOCK_INDUSTRY_ID, &industry, STOCK_URL_ID, &url, -1);

			// Get quotes file name
			gchar* path = GetQuotesFile (file_name, ticker);

			// Run edit quotes dialog
			status = ViewQuotesDialog (GTK_WINDOW (window), path, ticker, name, country, sector, industry, url);

			// Free temporary string buffers
			g_free (ticker);
			g_free (name);
			g_free (country);
			g_free (sector);
			g_free (industry);
			g_free (url);
			g_free (path);
		}
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Edit" menu button                                 //
//****************************************************************************//
static gboolean EditQuotes (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree view selection
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

	// Check if any stock is selected
	GtkTreeModel *model;
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), &model, &iter))
	{
		// Check if stock list is saved
		if (file_name == NULL)
			AskToSaveStockList ();
		else
		{
			// Get stock details
			gchar *ticker;
			gchar *name;
			gchar *country;
			gchar *sector;
			gchar *industry;
			gchar *url;
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, STOCK_TICKER_ID, &ticker, STOCK_NAME_ID, &name, STOCK_COUNTRY_ID, &country, STOCK_SECTOR_ID, &sector, STOCK_INDUSTRY_ID, &industry, STOCK_URL_ID, &url, -1);

			// Get quotes file name
			gchar* path = GetQuotesFile (file_name, ticker);

			// Run edit quotes dialog
			status = EditQuotesDialog (GTK_WINDOW (window), path, ticker, name, country, sector, industry, url);

			// Free temporary string buffers
			g_free (ticker);
			g_free (name);
			g_free (country);
			g_free (sector);
			g_free (industry);
			g_free (url);
			g_free (path);
		}
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "About" menu button                                //
//****************************************************************************//
static gboolean AboutDialog (void)
{
	// Set authors array
	const gchar* authors[] = {"Jack Black <ezamlinsky@gmail.com>", NULL};

	// Show about dialog window
	gtk_show_about_dialog (GTK_WINDOW (window), "program-name", PROGRAM_NAME, "version", "1.0", "comments", "These filters select only good stocks for trading", "website-label", "LinAsm website", "website", "http://linasm.sourceforge.net/", "copyright", "Copyright (C) 2014, Jack Black", "license-type", GTK_LICENSE_GPL_3_0, "authors", authors, "logo", logo, NULL);

	// Return success state
	return TRUE;
}

//****************************************************************************//
//      Signal handler for selection changed                                  //
//****************************************************************************//
static void SelectionChanged (GtkTreeSelection *selection, gpointer user_data)
{
	// Change selection state
	ChangeSelectionState (gtk_tree_selection_count_selected_rows (GTK_TREE_SELECTION (selection)));
}

//****************************************************************************//
//      Signal handler for column header click                                //
//****************************************************************************//
static void ColumnHandler (GtkTreeViewColumn *column, gpointer data)
{
	// Set search column for immediate search
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), GPOINTER_TO_UINT (data));
}

//****************************************************************************//
//      Signal handler for "Check" cell                                       //
//****************************************************************************//
static void CheckCellHandler (GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path))
		{
			// Get mark state
			gboolean state;
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, GPOINTER_TO_UINT (data), &state, -1);

			// Switch mark state
			gtk_list_store_set (GTK_LIST_STORE (model), &iter, GPOINTER_TO_UINT (data), !state, -1);
		}
	}
}

//****************************************************************************//
//      Signal handler for "Ticker" cell                                      //
//****************************************************************************//
static void TickerCellHandler (GtkCellRendererText *cell, gchar *path, gchar *newval, gpointer data)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path))
		{
			// Get old ticker value
			gchar *oticker;
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, GPOINTER_TO_UINT (data), &oticker, -1);

			// Create error object
			GError *error = NULL;

			// Check if new ticker is not empty
			if (!CheckField (newval, "ticker", &error))
				ShowErrorMessage (GTK_WINDOW (window), "Incorrect stock ticker", error);
			else
			{
				// Strip new ticker
				newval = g_strstrip (newval);

				// Convert new ticker to upper case
				gchar *upper = g_utf8_strup (newval, -1);

				// Check if ticker is changed
				if (g_utf8_collate (oticker, upper))
				{
					// Check if new ticker unique
					if (!IsTickerUnique (GTK_TREE_MODEL (model), upper, &error))
						ShowErrorMessage (GTK_WINDOW (window), "Duplicate dates are not allowed", error);
					else
					{
						// Set value
						gtk_list_store_set (GTK_LIST_STORE (model), &iter, GPOINTER_TO_UINT (data), upper, -1);

						// Change save state
						ChangeSaveState (FALSE);
					}
				}

				// Free temporary string buffer
				g_free (upper);
			}

			// Free temporary string buffer
			g_free (oticker);
		}
	}
}

//****************************************************************************//
//      Signal handler for "Name" cell                                        //
//****************************************************************************//
static void NameCellHandler (GtkCellRendererText *cell, gchar *path, gchar *newval, gpointer data)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path))
		{
			// Get old name value
			gchar *oname;
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, GPOINTER_TO_UINT (data), &oname, -1);

			// Create error object
			GError *error = NULL;

			// Check if new name is not empty
			if (!CheckField (newval, "name", &error))
				ShowErrorMessage (GTK_WINDOW (window), "Incorrect stock name", error);
			else
			{
				// Strip new name
				newval = g_strstrip (newval);

				// Check if name is changed
				if (g_utf8_collate (oname, newval))
				{
					// Set value
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, GPOINTER_TO_UINT (data), newval, -1);

					// Change save state
					ChangeSaveState (FALSE);
				}
			}

			// Free temporary string buffer
			g_free (oname);
		}
	}
}

//****************************************************************************//
//      Signal handler for column cell                                        //
//****************************************************************************//
static void CellHandler (GtkCellRendererText *cell, gchar *path, gchar *newval, gpointer data)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path))
		{
			// Get old value
			gchar *oldval;
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, GPOINTER_TO_UINT (data), &oldval, -1);

			// Strip new name
			newval = g_strstrip (newval);

			// Check if value is changed
			if (g_utf8_collate (oldval, newval))
			{
				// Set value
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, GPOINTER_TO_UINT (data), newval, -1);

				// Change save state
				ChangeSaveState (FALSE);
			}

			// Free temporary string buffer
			g_free (oldval);
		}
	}
}

//****************************************************************************//
//      "File" menu                                                           //
//****************************************************************************//

//============================================================================//
//      Submenu buttons                                                       //
//============================================================================//
static void CreateFileSubMenu (void)
{
	// Create submenu
	filemenu = gtk_menu_new ();

	// Create submenu elements
	GtkWidget *New = gtk_menu_item_new_with_mnemonic (MENU_FILE_NEW);
	GtkWidget *Open = gtk_menu_item_new_with_mnemonic (MENU_FILE_OPEN);
	GtkWidget *Separator1 = gtk_separator_menu_item_new ();
	GtkWidget *Save = gtk_menu_item_new_with_mnemonic (MENU_FILE_SAVE);
	GtkWidget *SaveAs = gtk_menu_item_new_with_mnemonic (MENU_FILE_SAVE_AS);
	GtkWidget *Separator2 = gtk_separator_menu_item_new ();
	GtkWidget *Import = gtk_menu_item_new_with_mnemonic (MENU_FILE_IMPORT);
	GtkWidget *Export = gtk_menu_item_new_with_mnemonic (MENU_FILE_EXPORT);
	GtkWidget *Separator3 = gtk_separator_menu_item_new ();
	GtkWidget *Properties = gtk_menu_item_new_with_mnemonic (MENU_FILE_PROPERTIES);
	GtkWidget *Separator4 = gtk_separator_menu_item_new ();
	GtkWidget *Close = gtk_menu_item_new_with_mnemonic (MENU_FILE_CLOSE);
	GtkWidget *Quit = gtk_menu_item_new_with_mnemonic (MENU_FILE_QUIT);

	// Add elements to submenu
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (New));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Open));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Separator1));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Save));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (SaveAs));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Separator2));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Import));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Export));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Separator3));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Properties));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Separator4));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Close));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Quit));

	// Add accelerators to menu buttons
	gtk_widget_add_accelerator (GTK_WIDGET (New), "activate", GTK_ACCEL_GROUP (accelgroup), 'N', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Open), "activate", GTK_ACCEL_GROUP (accelgroup), 'O', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Save), "activate", GTK_ACCEL_GROUP (accelgroup), 'S', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Properties), "activate", GTK_ACCEL_GROUP (accelgroup), 'P', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Close), "activate", GTK_ACCEL_GROUP (accelgroup), 'W', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Quit), "activate", GTK_ACCEL_GROUP (accelgroup), 'Q', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// Set submenu element properties
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (New), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Open), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Save), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (SaveAs), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Import), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Export), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Properties), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Close), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Quit), TRUE);

	// Assign signal handlers for "select" signal
	g_signal_connect (G_OBJECT (New), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Create new stock list"));
	g_signal_connect (G_OBJECT (Open), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Open existing stock file"));
	g_signal_connect (G_OBJECT (Save), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Save stocks into file"));
	g_signal_connect (G_OBJECT (SaveAs), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Save stocks into new file"));
	g_signal_connect (G_OBJECT (Import), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Import stocks from TSV file"));
	g_signal_connect (G_OBJECT (Export), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Export stocks into TSV file"));
	g_signal_connect (G_OBJECT (Properties), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("View stock file properties"));
	g_signal_connect (G_OBJECT (Close), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Close opened stock file"));
	g_signal_connect (G_OBJECT (Quit), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Quit the application"));

	// Assign signal handlers for "deselect" signal
	g_signal_connect (G_OBJECT (New), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Open), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Save), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (SaveAs), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Import), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Export), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Properties), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Close), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Quit), "deselect", G_CALLBACK (MenuDeselect), NULL);

	// Assign signal handlers for "activate" signal
	g_signal_connect (G_OBJECT (New), "activate", G_CALLBACK (NewStocks), NULL);
	g_signal_connect (G_OBJECT (Open), "activate", G_CALLBACK (OpenStocks), NULL);
	g_signal_connect (G_OBJECT (Save), "activate", G_CALLBACK (SaveStocks), NULL);
	g_signal_connect (G_OBJECT (SaveAs), "activate", G_CALLBACK (SaveStocksAs), NULL);
	g_signal_connect (G_OBJECT (Import), "activate", G_CALLBACK (ImportStocks), NULL);
	g_signal_connect (G_OBJECT (Export), "activate", G_CALLBACK (ExportStocks), NULL);
	g_signal_connect (G_OBJECT (Properties), "activate", G_CALLBACK (DocumentProperties), NULL);
	g_signal_connect (G_OBJECT (Close), "activate", G_CALLBACK (CloseDocument), NULL);
	g_signal_connect (G_OBJECT (Quit), "activate", G_CALLBACK (QuitProgram), NULL);
}

//============================================================================//
//      Menu button                                                           //
//============================================================================//
static GtkWidget* CreateFileMenu (void)
{
	// Create menu
	GtkWidget *menu = gtk_menu_item_new_with_mnemonic ("_File");

	// Create submenu
	CreateFileSubMenu ();

	// Add submenu
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), GTK_WIDGET (filemenu));

	// Return menu object
	return menu;
}

//****************************************************************************//
//      "Edit" menu                                                           //
//****************************************************************************//

//============================================================================//
//      Submenu buttons                                                       //
//============================================================================//
static void CreateEditSubMenu (void)
{
	// Create submenu
	editmenu = gtk_menu_new ();

	// Create submenu elements
	GtkWidget *Insert = gtk_menu_item_new_with_mnemonic (MENU_EDIT_INSERT);
	GtkWidget *Remove = gtk_menu_item_new_with_mnemonic (MENU_EDIT_REMOVE);
	GtkWidget *Separator1 = gtk_separator_menu_item_new ();
	GtkWidget *SelectAll = gtk_menu_item_new_with_mnemonic (MENU_EDIT_SELECT_ALL);
	GtkWidget *Invert = gtk_menu_item_new_with_mnemonic (MENU_EDIT_INVERT_SELECTION);
	GtkWidget *UnselectAll = gtk_menu_item_new_with_mnemonic (MENU_EDIT_UNSELECT_ALL);
	GtkWidget *Separator2 = gtk_separator_menu_item_new ();
	GtkWidget *Find = gtk_menu_item_new_with_mnemonic (MENU_EDIT_FIND);
	GtkWidget *Sort = gtk_menu_item_new_with_mnemonic (MENU_EDIT_SORT);

	// Add elements to submenu
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (Insert));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (Remove));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (Separator1));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (SelectAll));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (Invert));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (UnselectAll));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (Separator2));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (Find));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (Sort));

	// Add accelerators to menu buttons
	gtk_widget_add_accelerator (GTK_WIDGET (Insert), "activate", GTK_ACCEL_GROUP (accelgroup), 'I', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Remove), "activate", GTK_ACCEL_GROUP (accelgroup), 'R', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (SelectAll), "activate", GTK_ACCEL_GROUP (accelgroup), 'A', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Invert), "activate", GTK_ACCEL_GROUP (accelgroup), 'X', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (UnselectAll), "activate", GTK_ACCEL_GROUP (accelgroup), 'U', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Find), "activate", GTK_ACCEL_GROUP (accelgroup), 'F', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Sort), "activate", GTK_ACCEL_GROUP (accelgroup), 'T', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// Set submenu element properties
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Insert), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Remove), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (SelectAll), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Invert), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (UnselectAll), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Find), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Sort), TRUE);

	// Assign signal handlers for "select" signal
	g_signal_connect (G_OBJECT (Insert), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Insert new stock into stock list"));
	g_signal_connect (G_OBJECT (Remove), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Remove selected stock from stock list"));
	g_signal_connect (G_OBJECT (SelectAll), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Select all stocks"));
	g_signal_connect (G_OBJECT (Invert), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Invert selection mark for all stocks"));
	g_signal_connect (G_OBJECT (UnselectAll), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Unselect all stocks"));
	g_signal_connect (G_OBJECT (Find), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Find stocks in the list by attribute"));
	g_signal_connect (G_OBJECT (Sort), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Sort stock list by attribute"));

	// Assign signal handlers for "deselect" signal
	g_signal_connect (G_OBJECT (Insert), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Remove), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (SelectAll), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Invert), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (UnselectAll), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Find), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Sort), "deselect", G_CALLBACK (MenuDeselect), NULL);

	// Assign signal handlers for "activate" signal
	g_signal_connect (G_OBJECT (Insert), "activate", G_CALLBACK (InsertStock), NULL);
	g_signal_connect (G_OBJECT (Remove), "activate", G_CALLBACK (RemoveStocks), NULL);
	g_signal_connect (G_OBJECT (SelectAll), "activate", G_CALLBACK (SelectAllStocks), NULL);
	g_signal_connect (G_OBJECT (Invert), "activate", G_CALLBACK (InvertSelection), NULL);
	g_signal_connect (G_OBJECT (UnselectAll), "activate", G_CALLBACK (UnselectAllStocks), NULL);
	g_signal_connect (G_OBJECT (Find), "activate", G_CALLBACK (FindStocks), NULL);
	g_signal_connect (G_OBJECT (Sort), "activate", G_CALLBACK (SortStocks), NULL);
}

//============================================================================//
//      Menu button                                                           //
//============================================================================//
static GtkWidget* CreateEditMenu (void)
{
	// Create menu
	GtkWidget *menu = gtk_menu_item_new_with_mnemonic ("_Edit");

	// Create submenu
	CreateEditSubMenu ();

	// Add submenu
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), GTK_WIDGET (editmenu));

	// Return menu object
	return menu;
}

//****************************************************************************//
//      "Stocks" menu                                                         //
//****************************************************************************//

//============================================================================//
//      Submenu buttons                                                       //
//============================================================================//
static void CreateStocksSubMenu (void)
{
	// Create submenu
	stocksmenu = gtk_menu_new ();

	// Create submenu elements
	GtkWidget *Info = gtk_menu_item_new_with_mnemonic (MENU_STOCKS_INFO);
	GtkWidget *Check = gtk_menu_item_new_with_mnemonic (MENU_STOCKS_CHECK);
	GtkWidget *Sync = gtk_menu_item_new_with_mnemonic (MENU_STOCKS_SYNC);
	GtkWidget *Analyze = gtk_menu_item_new_with_mnemonic (MENU_STOCKS_ANALYZE);

	// Add elements to submenu
	gtk_menu_shell_append (GTK_MENU_SHELL (stocksmenu), GTK_WIDGET (Info));
	gtk_menu_shell_append (GTK_MENU_SHELL (stocksmenu), GTK_WIDGET (Check));
	gtk_menu_shell_append (GTK_MENU_SHELL (stocksmenu), GTK_WIDGET (Sync));
	gtk_menu_shell_append (GTK_MENU_SHELL (stocksmenu), GTK_WIDGET (Analyze));

	// Add accelerators to menu buttons
	gtk_widget_add_accelerator (GTK_WIDGET (Info), "activate", GTK_ACCEL_GROUP (accelgroup), 'D', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Check), "activate", GTK_ACCEL_GROUP (accelgroup), 'C', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Sync), "activate", GTK_ACCEL_GROUP (accelgroup), 'Y', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Analyze), "activate", GTK_ACCEL_GROUP (accelgroup), 'L', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// Set submenu element properties
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Info), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Check), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Sync), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Analyze), TRUE);

	// Assign signal handlers for "select" signal
	g_signal_connect (G_OBJECT (Info), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Show stock details"));
	g_signal_connect (G_OBJECT (Check), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Check stock quotes for errors"));
	g_signal_connect (G_OBJECT (Sync), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Sync stock quotes with quotes server"));
	g_signal_connect (G_OBJECT (Analyze), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Analyze stocks quotes for trading"));

	// Assign signal handlers for "deselect" signal
	g_signal_connect (G_OBJECT (Info), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Check), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Sync), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Analyze), "deselect", G_CALLBACK (MenuDeselect), NULL);

	// Assign signal handlers for "activate" signal
	g_signal_connect (G_OBJECT (Info), "activate", G_CALLBACK (StockInfo), NULL);
	g_signal_connect (G_OBJECT (Check), "activate", G_CALLBACK (CheckStocks), NULL);
	g_signal_connect (G_OBJECT (Sync), "activate", G_CALLBACK (SyncStocks), NULL);
	g_signal_connect (G_OBJECT (Analyze), "activate", G_CALLBACK (AnalyzeStocks), NULL);
}

//============================================================================//
//      Menu button                                                           //
//============================================================================//
static GtkWidget* CreateStocksMenu (void)
{
	// Create menu
	GtkWidget *menu = gtk_menu_item_new_with_mnemonic ("_Stocks");

	// Create submenu
	CreateStocksSubMenu ();

	// Add submenu
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), GTK_WIDGET (stocksmenu));

	// Return menu object
	return menu;
}

//****************************************************************************//
//      "Quotes" menu                                                         //
//****************************************************************************//

//============================================================================//
//      Submenu buttons                                                       //
//============================================================================//
static void CreateQuotesSubMenu (void)
{
	// Create submenu
	quotesmenu = gtk_menu_new ();

	// Create submenu elements
	GtkWidget *View = gtk_menu_item_new_with_mnemonic (MENU_QUOTES_VIEW);
	GtkWidget *Edit = gtk_menu_item_new_with_mnemonic (MENU_QUOTES_EDIT);

	// Add elements to submenu
	gtk_menu_shell_append (GTK_MENU_SHELL (quotesmenu), GTK_WIDGET (View));
	gtk_menu_shell_append (GTK_MENU_SHELL (quotesmenu), GTK_WIDGET (Edit));

	// Add accelerators to menu buttons
	gtk_widget_add_accelerator (GTK_WIDGET (View), "activate", GTK_ACCEL_GROUP (accelgroup), 'V', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Edit), "activate", GTK_ACCEL_GROUP (accelgroup), 'E', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// Set submenu element properties
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (View), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Edit), TRUE);

	// Assign signal handlers for "select" signal
	g_signal_connect (G_OBJECT (View), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("View stock quotes and quotes chart"));
	g_signal_connect (G_OBJECT (Edit), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Edit stock quotes"));

	// Assign signal handlers for "deselect" signal
	g_signal_connect (G_OBJECT (View), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Edit), "deselect", G_CALLBACK (MenuDeselect), NULL);

	// Assign signal handlers for "activate" signal
	g_signal_connect (G_OBJECT (View), "activate", G_CALLBACK (ViewQuotes), NULL);
	g_signal_connect (G_OBJECT (Edit), "activate", G_CALLBACK (EditQuotes), NULL);
}

//============================================================================//
//      Menu button                                                           //
//============================================================================//
static GtkWidget* CreateQuotesMenu (void)
{
	// Create menu
	GtkWidget *menu = gtk_menu_item_new_with_mnemonic ("_Quotes");

	// Create submenu
	CreateQuotesSubMenu ();

	// Add submenu
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), GTK_WIDGET (quotesmenu));

	// Return menu object
	return menu;
}

//****************************************************************************//
//      "Help" menu                                                           //
//****************************************************************************//

//============================================================================//
//      Submenu buttons                                                       //
//============================================================================//
static void CreateHelpSubMenu (void)
{
	// Create submenu
	helpmenu = gtk_menu_new ();

	// Create submenu elements
	GtkWidget *About = gtk_menu_item_new_with_mnemonic (MENU_HELP_ABOUT);

	// Add elements to submenu
	gtk_menu_shell_append (GTK_MENU_SHELL (helpmenu), GTK_WIDGET (About));

	// Set submenu element properties
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (About), TRUE);

	// Assign signal handlers for "select" signal
	g_signal_connect (G_OBJECT (About), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Information about stock filter program"));

	// Assign signal handlers for "deselect" signal
	g_signal_connect (G_OBJECT (About), "deselect", G_CALLBACK (MenuDeselect), NULL);

	// Assign signal handlers for "activate" signal
	g_signal_connect (G_OBJECT (About), "activate", G_CALLBACK (AboutDialog), NULL);
}

//============================================================================//
//      Menu button                                                           //
//============================================================================//
static GtkWidget* CreateHelpMenu (void)
{
	// Create menu
	GtkWidget *menu = gtk_menu_item_new_with_mnemonic ("_Help");

	// Create submenu
	CreateHelpSubMenu ();

	// Add submenu
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), GTK_WIDGET (helpmenu));

	// Return menu object
	return menu;
}

//****************************************************************************//
//      Menu bar                                                              //
//****************************************************************************//
static GtkWidget* CreateMenuBar (void)
{
	// Create menu bar
	GtkWidget *menubar = gtk_menu_bar_new ();

	// Add menu buttons
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), GTK_WIDGET (CreateFileMenu ()));
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), GTK_WIDGET (CreateEditMenu ()));
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), GTK_WIDGET (CreateStocksMenu ()));
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), GTK_WIDGET (CreateQuotesMenu ()));
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), GTK_WIDGET (CreateHelpMenu ()));

	// Return menu bar object
	return menubar;
}

//****************************************************************************//
//      Tool bar                                                              //
//****************************************************************************//
static GtkWidget* CreateToolBar (void)
{
	// Create tool bar
	toolbar = gtk_toolbar_new ();

	// Create buttons
	GtkToolItem *New = gtk_tool_button_new (NULL, STOCK_TOOL_NEW);
	GtkToolItem *Open = gtk_tool_button_new (NULL, STOCK_TOOL_OPEN);
	GtkToolItem *Save = gtk_tool_button_new (NULL, STOCK_TOOL_SAVE);
	GtkToolItem *Separator1 = gtk_separator_tool_item_new();
	GtkToolItem *Import = gtk_tool_button_new (NULL, STOCK_TOOL_IMPORT);
	GtkToolItem *Export = gtk_tool_button_new (NULL, STOCK_TOOL_EXPORT);
	GtkToolItem *Separator2 = gtk_separator_tool_item_new();
	GtkToolItem *Insert = gtk_tool_button_new (NULL, STOCK_TOOL_INSERT);
	GtkToolItem *Remove = gtk_tool_button_new (NULL, STOCK_TOOL_REMOVE);
	GtkToolItem *Separator3 = gtk_separator_tool_item_new();
	GtkToolItem *All = gtk_tool_button_new (NULL, STOCK_TOOL_ALL);
	GtkToolItem *None = gtk_tool_button_new (NULL, STOCK_TOOL_NONE);
	GtkToolItem *Separator4 = gtk_separator_tool_item_new();
	GtkToolItem *Find = gtk_tool_button_new (NULL, STOCK_TOOL_FIND);
	GtkToolItem *Separator5 = gtk_separator_tool_item_new();
	GtkToolItem *Check = gtk_tool_button_new (NULL, STOCK_TOOL_CHECK);
	GtkToolItem *Sync = gtk_tool_button_new (NULL, STOCK_TOOL_SYNC);
	GtkToolItem *Analyze = gtk_tool_button_new (NULL, STOCK_TOOL_ANALYZE);
	GtkToolItem *Separator6 = gtk_separator_tool_item_new();
	GtkToolItem *Quotes = gtk_tool_button_new (NULL, STOCK_TOOL_QUOTES);
	GtkToolItem *Separator7 = gtk_separator_tool_item_new();
	GtkToolItem *Info = gtk_tool_button_new (NULL, STOCK_TOOL_INFO);
	GtkToolItem *Quit = gtk_tool_button_new (NULL, STOCK_TOOL_QUIT);

	// Add buttons to tool bar
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (New), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Open), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Save), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Separator1), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Import), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Export), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Separator2), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Insert), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Remove), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Separator3), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (All), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (None), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Separator4), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Find), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Separator5), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Check), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Sync), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Analyze), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Separator6), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Quotes), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Separator7), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Info), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Quit), -1);

	// Add tooltip text to buttons
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (New), "Create new stock list");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Open), "Open existing stock file");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Save), "Save stocks into file");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Import), "Import stocks from TSV file");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Export), "Export stocks into TSV file");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Insert), "Insert new stock into stock list");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Remove), "Remove selected stocks from stock list");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (All), "Select all stocks");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (None), "Unselect all stocks");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Find), "Find stocks in the list by attribute");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Check), "Check stock quotes for errors");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Sync), "Sync stock quotes with quotes server");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Analyze), "Analyze stocks quotes for trading");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Quotes), "View stock quotes and quotes chart");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Info), "Show stock details");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Quit), "Quit the application");

	// Set button properties
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (New), "document-new");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Open), "document-open");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Save), "document-save");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Import), "document-import");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Export), "document-export");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Insert), "list-add");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Remove), "list-remove");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (All), "view-list-compact");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (None), "view-list-details");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Find), "edit-find");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Check), "dialog-ok");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Sync), "system-run");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Analyze), "media-playback-start");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Quotes), "document-properties");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Info), "gtk-info");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Quit), "application-exit");
	gtk_tool_item_set_expand (GTK_TOOL_ITEM (Separator7), TRUE);

	// Set tool bar properties
	gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), TRUE);
	gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH);
	gtk_toolbar_set_icon_size (GTK_TOOLBAR (toolbar), GTK_ICON_SIZE_LARGE_TOOLBAR);

	// Assign signal handlers
	g_signal_connect (G_OBJECT (New), "clicked", G_CALLBACK (NewStocks), NULL);
	g_signal_connect (G_OBJECT (Open), "clicked", G_CALLBACK (OpenStocks), NULL);
	g_signal_connect (G_OBJECT (Save), "clicked", G_CALLBACK (SaveStocks), NULL);
	g_signal_connect (G_OBJECT (Import), "clicked", G_CALLBACK (ImportStocks), NULL);
	g_signal_connect (G_OBJECT (Export), "clicked", G_CALLBACK (ExportStocks), NULL);
	g_signal_connect (G_OBJECT (Insert), "clicked", G_CALLBACK (InsertStock), NULL);
	g_signal_connect (G_OBJECT (Remove), "clicked", G_CALLBACK (RemoveStocks), NULL);
	g_signal_connect (G_OBJECT (All), "clicked", G_CALLBACK (SelectAllStocks), NULL);
	g_signal_connect (G_OBJECT (None), "clicked", G_CALLBACK (UnselectAllStocks), NULL);
	g_signal_connect (G_OBJECT (Find), "clicked", G_CALLBACK (FindStocks), NULL);
	g_signal_connect (G_OBJECT (Check), "clicked", G_CALLBACK (CheckStocks), NULL);
	g_signal_connect (G_OBJECT (Sync), "clicked", G_CALLBACK (SyncStocks), NULL);
	g_signal_connect (G_OBJECT (Analyze), "clicked", G_CALLBACK (AnalyzeStocks), NULL);
	g_signal_connect (G_OBJECT (Quotes), "clicked", G_CALLBACK (ViewQuotes), NULL);
	g_signal_connect (G_OBJECT (Info), "clicked", G_CALLBACK (StockInfo), NULL);
	g_signal_connect (G_OBJECT (Quit), "clicked", G_CALLBACK (QuitProgram), NULL);

	// Return tool bar
	return toolbar;
}

//****************************************************************************//
//      Stock list                                                            //
//****************************************************************************//
static GtkWidget* CreateStockList (void)
{
	// Create tree view
	treeview = gtk_tree_view_new ();

	// Get tree view adjustments
	GtkAdjustment *vadjustment = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (treeview));
	GtkAdjustment *hadjustment = gtk_scrollable_get_hadjustment (GTK_SCROLLABLE (treeview));

	// Get tree view selection
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

	// Create scrolled window
	GtkWidget *scrolled = gtk_scrolled_window_new (hadjustment, vadjustment);

	// Create cell renderers for columns
	GtkCellRenderer *CheckCell = gtk_cell_renderer_toggle_new ();
	GtkCellRenderer *TickerCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *NameCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *CountryCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *SectorCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *IndustryCell = gtk_cell_renderer_text_new ();

	// Create columns for tree view
	GtkTreeViewColumn *CheckColumn = gtk_tree_view_column_new_with_attributes (STOCK_CHECK_LABEL, GTK_CELL_RENDERER (CheckCell), "active", STOCK_CHECK_ID, NULL);
	GtkTreeViewColumn *TickerColumn = gtk_tree_view_column_new_with_attributes (STOCK_TICKER_LABEL, GTK_CELL_RENDERER (TickerCell), "text", STOCK_TICKER_ID, NULL);
	GtkTreeViewColumn *NameColumn = gtk_tree_view_column_new_with_attributes (STOCK_NAME_LABEL, GTK_CELL_RENDERER (NameCell), "text", STOCK_NAME_ID, NULL);
	GtkTreeViewColumn *CountryColumn = gtk_tree_view_column_new_with_attributes (STOCK_COUNTRY_LABEL, GTK_CELL_RENDERER (CountryCell), "text", STOCK_COUNTRY_ID, NULL);
	GtkTreeViewColumn *SectorColumn = gtk_tree_view_column_new_with_attributes (STOCK_SECTOR_LABEL, GTK_CELL_RENDERER (SectorCell), "text", STOCK_SECTOR_ID, NULL);
	GtkTreeViewColumn *IndustryColumn = gtk_tree_view_column_new_with_attributes (STOCK_INDUSTRY_LABEL, GTK_CELL_RENDERER (IndustryCell), "text", STOCK_INDUSTRY_ID, NULL);

	// Add columns to tree view
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (CheckColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (TickerColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (NameColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (CountryColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (SectorColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (IndustryColumn));

	// Add tree view to scrolled window
	gtk_container_add (GTK_CONTAINER (scrolled), GTK_WIDGET (treeview));

	// Set cell properties
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (TickerCell), 0.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (NameCell), 0.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (CountryCell), 0.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (SectorCell), 0.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (IndustryCell), 0.0, 0.0);
	g_object_set (G_OBJECT (TickerCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (NameCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (CountryCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (SectorCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (IndustryCell), "editable", TRUE, NULL);

	// Set column properties
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (CheckColumn), FALSE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (TickerColumn), FALSE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (NameColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (CountryColumn), FALSE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (SectorColumn), FALSE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (IndustryColumn), FALSE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (CheckColumn), FALSE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (TickerColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (NameColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (CountryColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (SectorColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (IndustryColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (CheckColumn), FALSE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (TickerColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (NameColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (CountryColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (SectorColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (IndustryColumn), TRUE);

	// Set tree view sort properties
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (CheckColumn), STOCK_CHECK_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (TickerColumn), STOCK_TICKER_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (NameColumn), STOCK_NAME_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (CountryColumn), STOCK_COUNTRY_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (SectorColumn), STOCK_SECTOR_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (IndustryColumn), STOCK_INDUSTRY_ID);

	// Set tree view properties
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_GRID_LINES_NONE);

	// Set scrolled window preperties
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_placement (GTK_SCROLLED_WINDOW (scrolled), GTK_CORNER_TOP_LEFT);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled), GTK_SHADOW_NONE);
	gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (scrolled), 1200);
	gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (scrolled), 600);

	// Assign signal handlers for selection
	g_signal_connect (G_OBJECT (selection), "changed", G_CALLBACK (SelectionChanged), NULL);

	// Assign signal handlers for cells
	g_signal_connect (G_OBJECT (CheckCell), "toggled", G_CALLBACK (CheckCellHandler), GUINT_TO_POINTER (STOCK_CHECK_ID));
	g_signal_connect (G_OBJECT (TickerCell), "edited", G_CALLBACK (TickerCellHandler), GUINT_TO_POINTER (STOCK_TICKER_ID));
	g_signal_connect (G_OBJECT (NameCell), "edited", G_CALLBACK (NameCellHandler), GUINT_TO_POINTER (STOCK_NAME_ID));
	g_signal_connect (G_OBJECT (CountryCell), "edited", G_CALLBACK (CellHandler), GUINT_TO_POINTER (STOCK_COUNTRY_ID));
	g_signal_connect (G_OBJECT (SectorCell), "edited", G_CALLBACK (CellHandler), GUINT_TO_POINTER (STOCK_SECTOR_ID));
	g_signal_connect (G_OBJECT (IndustryCell), "edited", G_CALLBACK (CellHandler), GUINT_TO_POINTER (STOCK_INDUSTRY_ID));

	// Assign signal handlers for columns
	g_signal_connect (G_OBJECT (TickerColumn), "clicked", G_CALLBACK (ColumnHandler), GUINT_TO_POINTER (STOCK_TICKER_ID));
	g_signal_connect (G_OBJECT (NameColumn), "clicked", G_CALLBACK (ColumnHandler), GUINT_TO_POINTER (STOCK_NAME_ID));
	g_signal_connect (G_OBJECT (CountryColumn), "clicked", G_CALLBACK (ColumnHandler), GUINT_TO_POINTER (STOCK_COUNTRY_ID));
	g_signal_connect (G_OBJECT (SectorColumn), "clicked", G_CALLBACK (ColumnHandler), GUINT_TO_POINTER (STOCK_SECTOR_ID));
	g_signal_connect (G_OBJECT (IndustryColumn), "clicked", G_CALLBACK (ColumnHandler), GUINT_TO_POINTER (STOCK_INDUSTRY_ID));

	// Return scrolled window object
	return scrolled;
}

//****************************************************************************//
//      Status bar                                                            //
//****************************************************************************//
static GtkWidget* CreateStatusBar (void)
{
	// Create status bar
	statusbar = gtk_statusbar_new ();

	// Get context identifier for menu messages
	menuid = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "Menu");
	fileid = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "File");

	// Return status bar object
	return statusbar;
}

//****************************************************************************//
//      Layout                                                                //
//****************************************************************************//
static GtkWidget* CreateLayout (void)
{
	// Create box
	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

	// Add items to box
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateMenuBar ()), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateToolBar ()), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateStockList ()), TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateStatusBar ()), FALSE, FALSE, 0);

	// Return box object
	return box;
}

//****************************************************************************//
//      Main function                                                         //
//****************************************************************************//
int main (int argc, char *argv[])
{
	// Create error object
	GError *error = NULL;

	// Init GTK library
	gtk_init (&argc, &argv);

	// Create main window
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	// Create accelerator group
	accelgroup = gtk_accel_group_new ();

	// Add accelerator group to window
	gtk_window_add_accel_group (GTK_WINDOW (window), GTK_ACCEL_GROUP (accelgroup));

	// Add items to main widow
	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (CreateLayout ()));

	// Load program logo
	logo = gdk_pixbuf_new_from_file (LOGO_FILE, &error);
	if (!logo)
		ShowStatusMessage (error -> message, fileid);

	// Set window properties
	gtk_window_set_title (GTK_WINDOW (window), PROGRAM_NAME);
	gtk_window_set_icon (GTK_WINDOW (window), GDK_PIXBUF (logo));
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
	gtk_window_set_decorated (GTK_WINDOW (window), TRUE);
	gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
	gtk_window_set_focus_visible (GTK_WINDOW (window), FALSE);

	// Assign signal handlers
	g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (QuitProgram), NULL);

	// Set initial program state
	ChangeDocumentState (NULL, NULL, FALSE);

	// Show all window elements
	gtk_widget_show_all (GTK_WIDGET (window));

	// Start main GTK loop
	gtk_main ();

	// Normal exit
	return 0;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
