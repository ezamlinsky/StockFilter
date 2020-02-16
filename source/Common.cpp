/*                                                                    Common.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                               COMMON FUNCTIONS                               #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# include	<StockList.h>
# include	<Common.h>
# include	<Time.h>

//****************************************************************************//
//      Internal constants                                                    //
//****************************************************************************//
# define	BUFFER_SIZE		512			// Static buffer size

//****************************************************************************//
//      Global objects                                                        //
//****************************************************************************//
extern	GtkWidget	*statusbar;

//****************************************************************************//
//      Global variables                                                      //
//****************************************************************************//
extern	guint		fileid;

//****************************************************************************//
//      Date render function                                                  //
//****************************************************************************//
void DateRenderFunc (GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	// Allocate space for static buffer
	gchar buffer [BUFFER_SIZE];

	// Get quote date
	time_t date;
	gtk_tree_model_get (GTK_TREE_MODEL (model), iter, GPOINTER_TO_UINT (data), &date, -1);

	// Check quote date
	if (date == static_cast <time_t> (TIME_ERROR))
	{
		// Set cell text
		g_object_set (G_OBJECT (cell), "text", STRING_UNKNOWN, NULL);
	}
	else
	{
		// Format quote date into string buffer
		date_struct curdate = Time::ExtractDate (date);
		g_snprintf (buffer, BUFFER_SIZE, "%.4i-%.2d-%.2d", curdate.year, curdate.mon, curdate.day);

		// Set cell text
		g_object_set (G_OBJECT (cell), "text", buffer, NULL);

	}
}

//****************************************************************************//
//      Price render function                                                 //
//****************************************************************************//
void PriceRenderFunc (GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	// Allocate space for static buffer
	gchar buffer [BUFFER_SIZE];

	// Get quote price
	gfloat price;
	gtk_tree_model_get (GTK_TREE_MODEL (model), iter, GPOINTER_TO_UINT (data), &price, -1);

	// Check quote price
	if (price < 0)
	{
		// Set cell text
		g_object_set (G_OBJECT (cell), "text", STRING_UNKNOWN, NULL);
	}
	else
	{
		// Format quote price into string buffer
		g_snprintf (buffer, BUFFER_SIZE, "%.2f", price);

		// Set cell text
		g_object_set (G_OBJECT (cell), "text", buffer, NULL);
	}
}

//****************************************************************************//
//      Percent render function                                               //
//****************************************************************************//
void PercentRenderFunc (GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	// Allocate space for static buffer
	gchar buffer [BUFFER_SIZE];

	// Get percent value
	gfloat percent;
	gtk_tree_model_get (GTK_TREE_MODEL (model), iter, GPOINTER_TO_UINT (data), &percent, -1);

	// Check percent value
	if (percent < 0)
	{
		// Set cell text
		g_object_set (G_OBJECT (cell), "text", STRING_UNKNOWN, NULL);
	}
	else
	{
		// Format percent value into string buffer
		g_snprintf (buffer, BUFFER_SIZE, "%.2f%%", percent * 100);

		// Set cell text
		g_object_set (G_OBJECT (cell), "text", buffer, NULL);
	}
}

//****************************************************************************//
//      Count render function                                                 //
//****************************************************************************//
void CountRenderFunc (GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	// Get quote price
	gint count;
	gtk_tree_model_get (GTK_TREE_MODEL (model), iter, GPOINTER_TO_UINT (data), &count, -1);

	// Check quote price
	if (count < 0)
	{
		// Set cell text
		g_object_set (G_OBJECT (cell), "text", STRING_UNKNOWN, NULL);
	}
}

//****************************************************************************//
//      Show status message                                                   //
//****************************************************************************//
void ShowStatusMessage (const gchar *message, guint context)
{
	// Remove all messages from status bar
	gtk_statusbar_remove_all (GTK_STATUSBAR (statusbar), context);

	// Push new message into status bar stack
	gtk_statusbar_push (GTK_STATUSBAR (statusbar), context, message);
}

//****************************************************************************//
//      Show error message                                                    //
//****************************************************************************//
void ShowErrorMessage (GtkWindow *parent, const gchar *message, GError *error)
{
	// Create message dialog window
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (parent), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, message);

	// Set dialog message
	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), error -> message);

	// Run dialog window
	gtk_dialog_run (GTK_DIALOG (dialog));

	// Destroy dialog widget
	gtk_widget_destroy (GTK_WIDGET (dialog));

	// Release error object
	g_error_free (error);
}

//****************************************************************************//
//      Show file operation error message                                     //
//****************************************************************************//
void ShowFileErrorMessage (GtkWindow *parent, const gchar *message, GError *error)
{
	// Show status message
	ShowStatusMessage (error -> message, fileid);

	// Show error message
	ShowErrorMessage (parent, message, error);
}

//****************************************************************************//
//      Get total count of slected stocks                                     //
//****************************************************************************//
gint GetTotalCount (GtkTreeModel *model)
{
	// Total stock count
	gint total = 0;

	// Get iterator position
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	{
			// Iterate through all elements
		do {
			// Get old state
			gboolean state;
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, STOCK_CHECK_ID, &state, -1);

			// Increment count of selected stocks
			if (state)
				total++;

			// Change iterator position to next element
		} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
	}

	// Return total stocks count
	return total;
}

//****************************************************************************//
//      Get quotes file for chosen stock                                      //
//****************************************************************************//
gchar* GetQuotesFile (const gchar *path, const gchar *ticker)
{
	// Truncate stock file path
	gchar *temp = g_strdup (path);
	*(g_utf8_strrchr (temp, -1, '.')) = '\0';

	// Create full path string to quotes file
	gchar *result = g_strconcat (temp, "/", ticker, ".hst", NULL);

	// Free temporary string buffer
	g_free (temp);

	// Return quote file path
	return result;
}

//****************************************************************************//
//      Signal handler for "Mark" tool buttons                                //
//****************************************************************************//
static void Mark (GtkToolButton *toolbutton, gpointer data)
{
	// Unselect all stocks
	UnselectAllStocks ();

	// Convert data pointer
	StatusList *list = reinterpret_cast <StatusList*> (data);

	// Process all reference list elements
	GList *element = g_list_first (list -> list);
	while (element)
	{
		// Get tree path
		GtkTreePath *path = gtk_tree_row_reference_get_path (reinterpret_cast <GtkTreeRowReference*> (element -> data));
		if (path)
		{
			// Get iterator position
			GtkTreeIter iter;
			if (gtk_tree_model_get_iter (GTK_TREE_MODEL (list -> model), &iter, path))
			{
				// Set mark state
				gtk_list_store_set (GTK_LIST_STORE (list -> model), &iter, STOCK_CHECK_ID, TRUE, -1);
			}
		}

		// Go to next reference list element
		element = element -> next;
	}
}

//****************************************************************************//
//      Tool bar                                                              //
//****************************************************************************//
static GtkWidget* CreateToolBar (StatusList *good, StatusList *bad)
{
	// Create tool bar
	GtkWidget *toolbar = gtk_toolbar_new ();

	// Create buttons
	GtkToolItem *Good = gtk_tool_button_new (NULL, "Mark good");
	GtkToolItem *Error = gtk_tool_button_new (NULL, "Mark error");

	// Add buttons to tool bar
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Good), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Error), -1);

	// Add tooltip text to buttons
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Good), "Mark good stocks");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Error), "Mark only stocks with errors");
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (Good), TRUE);
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (Error), TRUE);

	// Set button properties
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Good), "dialog-yes");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Error), "dialog-no");

	// Set tool bar properties
	gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), TRUE);
	gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH_HORIZ);
	gtk_toolbar_set_icon_size (GTK_TOOLBAR (toolbar), GTK_ICON_SIZE_LARGE_TOOLBAR);

	// Assign signal handlers
	g_signal_connect (G_OBJECT (Good), "clicked", G_CALLBACK (Mark), good);
	g_signal_connect (G_OBJECT (Error), "clicked", G_CALLBACK (Mark), bad);

	// Return tool bar
	return toolbar;
}

//****************************************************************************//
//      Report summary                                                        //
//****************************************************************************//
static GtkWidget* CreateReportSummary (gint total, gint errors, guint box_border, guint action_border)
{
	// Create box
	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

	// Create grids
	GtkWidget *lgrid = gtk_grid_new ();
	GtkWidget *rgrid = gtk_grid_new ();

	// Create label fields
	GtkWidget *TotalLabel = gtk_label_new (NULL);
	GtkWidget *ErrorsLabel = gtk_label_new (NULL);
	GtkWidget *TotalValue = gtk_label_new (NULL);
	GtkWidget *ErrorsValue = gtk_label_new (NULL);

	// Add labels to grids
	gtk_grid_attach (GTK_GRID (lgrid), GTK_WIDGET (TotalLabel), 0, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (rgrid), GTK_WIDGET (ErrorsLabel), 0, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (lgrid), GTK_WIDGET (TotalValue), 1, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (rgrid), GTK_WIDGET (ErrorsValue), 1, 0, 1, 1);

	// Add grids to box
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (lgrid), TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (rgrid), TRUE, TRUE, 0);

	// Set label properties
	gtk_label_set_selectable (GTK_LABEL (TotalLabel), FALSE);
	gtk_label_set_selectable (GTK_LABEL (ErrorsLabel), FALSE);
	gtk_label_set_selectable (GTK_LABEL (TotalValue), TRUE);
	gtk_label_set_selectable (GTK_LABEL (ErrorsValue), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (TotalLabel), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (ErrorsLabel), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (TotalValue), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (ErrorsValue), TRUE);
	gtk_widget_set_halign (GTK_WIDGET (TotalLabel), GTK_ALIGN_END);
	gtk_widget_set_halign (GTK_WIDGET (ErrorsLabel), GTK_ALIGN_END);
	gtk_widget_set_halign (GTK_WIDGET (TotalValue), GTK_ALIGN_START);
	gtk_widget_set_halign (GTK_WIDGET (ErrorsValue), GTK_ALIGN_START);

	// Set grid properties
	gtk_container_set_border_width (GTK_CONTAINER (lgrid), action_border);
	gtk_container_set_border_width (GTK_CONTAINER (rgrid), action_border);
	gtk_grid_set_row_spacing (GTK_GRID (lgrid), box_border);
	gtk_grid_set_row_spacing (GTK_GRID (rgrid), box_border);
	gtk_grid_set_column_spacing (GTK_GRID (lgrid), box_border + action_border);
	gtk_grid_set_column_spacing (GTK_GRID (rgrid), box_border + action_border);

	// Create string buffers
	GString *rstring = g_string_new (NULL);
	GString *estring = g_string_new (NULL);

	// Convert values into strings
	g_string_printf (rstring, "%i records", total);
	g_string_printf (estring, "%i errors", errors);

	// Set report details
	gtk_label_set_markup (GTK_LABEL (TotalLabel), "<b>Total records:</b>");
	gtk_label_set_markup (GTK_LABEL (ErrorsLabel), "<b>Errors found:</b>");
	gtk_label_set_text (GTK_LABEL (TotalValue), rstring -> str);
	gtk_label_set_text (GTK_LABEL (ErrorsValue), estring -> str);

	// Relase string buffers
	g_string_free (rstring, TRUE);
	g_string_free (estring, TRUE);

	// Return box object
	return box;
}

//****************************************************************************//
//      Stock summary                                                         //
//****************************************************************************//
GtkWidget* CreateStockSummary (const gchar *ticker, const gchar *name, const gchar *country, const gchar *sector, const gchar *industry, const gchar *url, guint box_border, guint action_border)
{
	// Create box
	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

	// Create grids
	GtkWidget *lgrid = gtk_grid_new ();
	GtkWidget *rgrid = gtk_grid_new ();

	// Create label fields
	GtkWidget *TickerLabel = gtk_label_new (NULL);
	GtkWidget *NameLabel = gtk_label_new (NULL);
	GtkWidget *CountryLabel = gtk_label_new (NULL);
	GtkWidget *SectorLabel = gtk_label_new (NULL);
	GtkWidget *IndustryLabel = gtk_label_new (NULL);
	GtkWidget *UrlLabel = gtk_label_new (NULL);
	GtkWidget *TickerValue = gtk_label_new (NULL);
	GtkWidget *NameValue = gtk_label_new (NULL);
	GtkWidget *CountryValue = gtk_label_new (NULL);
	GtkWidget *SectorValue = gtk_label_new (NULL);
	GtkWidget *IndustryValue = gtk_label_new (NULL);
	GtkWidget *UrlValue = gtk_label_new (NULL);

	// Add labels to grids
	gtk_grid_attach (GTK_GRID (lgrid), GTK_WIDGET (TickerLabel), 0, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (lgrid), GTK_WIDGET (NameLabel), 0, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (lgrid), GTK_WIDGET (CountryLabel), 0, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (rgrid), GTK_WIDGET (SectorLabel), 0, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (rgrid), GTK_WIDGET (IndustryLabel), 0, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (rgrid), GTK_WIDGET (UrlLabel), 0, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (lgrid), GTK_WIDGET (TickerValue), 1, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (lgrid), GTK_WIDGET (NameValue), 1, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (lgrid), GTK_WIDGET (CountryValue), 1, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (rgrid), GTK_WIDGET (SectorValue), 1, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (rgrid), GTK_WIDGET (IndustryValue), 1, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (rgrid), GTK_WIDGET (UrlValue), 1, 2, 1, 1);

	// Add grids to box
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (lgrid), TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (rgrid), TRUE, TRUE, 0);

	// Set label properties
	gtk_label_set_selectable (GTK_LABEL (TickerLabel), FALSE);
	gtk_label_set_selectable (GTK_LABEL (NameLabel), FALSE);
	gtk_label_set_selectable (GTK_LABEL (CountryLabel), FALSE);
	gtk_label_set_selectable (GTK_LABEL (SectorLabel), FALSE);
	gtk_label_set_selectable (GTK_LABEL (IndustryLabel), FALSE);
	gtk_label_set_selectable (GTK_LABEL (UrlLabel), FALSE);
	gtk_label_set_selectable (GTK_LABEL (TickerValue), TRUE);
	gtk_label_set_selectable (GTK_LABEL (NameValue), TRUE);
	gtk_label_set_selectable (GTK_LABEL (CountryValue), TRUE);
	gtk_label_set_selectable (GTK_LABEL (SectorValue), TRUE);
	gtk_label_set_selectable (GTK_LABEL (IndustryValue), TRUE);
	gtk_label_set_selectable (GTK_LABEL (UrlValue), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (TickerLabel), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (NameLabel), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (CountryLabel), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (SectorLabel), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (IndustryLabel), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (UrlLabel), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (TickerValue), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (NameValue), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (CountryValue), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (SectorValue), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (IndustryValue), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (UrlValue), TRUE);
	gtk_widget_set_halign (GTK_WIDGET (TickerLabel), GTK_ALIGN_END);
	gtk_widget_set_halign (GTK_WIDGET (NameLabel), GTK_ALIGN_END);
	gtk_widget_set_halign (GTK_WIDGET (CountryLabel), GTK_ALIGN_END);
	gtk_widget_set_halign (GTK_WIDGET (SectorLabel), GTK_ALIGN_END);
	gtk_widget_set_halign (GTK_WIDGET (IndustryLabel), GTK_ALIGN_END);
	gtk_widget_set_halign (GTK_WIDGET (UrlLabel), GTK_ALIGN_END);
	gtk_widget_set_halign (GTK_WIDGET (TickerValue), GTK_ALIGN_START);
	gtk_widget_set_halign (GTK_WIDGET (NameValue), GTK_ALIGN_START);
	gtk_widget_set_halign (GTK_WIDGET (CountryValue), GTK_ALIGN_START);
	gtk_widget_set_halign (GTK_WIDGET (SectorValue), GTK_ALIGN_START);
	gtk_widget_set_halign (GTK_WIDGET (IndustryValue), GTK_ALIGN_START);
	gtk_widget_set_halign (GTK_WIDGET (UrlValue), GTK_ALIGN_START);

	// Set grid properties
	gtk_container_set_border_width (GTK_CONTAINER (lgrid), action_border);
	gtk_container_set_border_width (GTK_CONTAINER (rgrid), action_border);
	gtk_grid_set_row_spacing (GTK_GRID (lgrid), box_border);
	gtk_grid_set_row_spacing (GTK_GRID (rgrid), box_border);
	gtk_grid_set_column_spacing (GTK_GRID (lgrid), box_border + action_border);
	gtk_grid_set_column_spacing (GTK_GRID (rgrid), box_border + action_border);

	// Set stock details
	gtk_label_set_markup (GTK_LABEL (TickerLabel), "<b>" STOCK_TICKER_LABEL ":</b>");
	gtk_label_set_markup (GTK_LABEL (NameLabel), "<b>" STOCK_NAME_LABEL ":</b>");
	gtk_label_set_markup (GTK_LABEL (CountryLabel), "<b>" STOCK_COUNTRY_LABEL ":</b>");
	gtk_label_set_markup (GTK_LABEL (SectorLabel), "<b>" STOCK_SECTOR_LABEL ":</b>");
	gtk_label_set_markup (GTK_LABEL (IndustryLabel), "<b>" STOCK_INDUSTRY_LABEL ":</b>");
	gtk_label_set_markup (GTK_LABEL (UrlLabel), "<b>" STOCK_URL_LABEL ":</b>");
	gtk_label_set_text (GTK_LABEL (TickerValue), ticker);
	gtk_label_set_text (GTK_LABEL (NameValue), name);
	gtk_label_set_text (GTK_LABEL (CountryValue), country);
	gtk_label_set_text (GTK_LABEL (SectorValue), sector);
	gtk_label_set_text (GTK_LABEL (IndustryValue), industry);
	gtk_label_set_text (GTK_LABEL (UrlValue), url);

	// Return box object
	return box;
}

//****************************************************************************//
//      Save report dialog                                                    //
//****************************************************************************//
static gboolean SaveReportDialog (GtkWindow *parent, const gchar *rname, GtkTreeModel *model, gboolean (*SaveFunction) (const gchar *fname, GtkTreeModel *model, GError **error))
{
	// Operation status
	gboolean status = FALSE;

	// Create dialog window
	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Save report to...", GTK_WINDOW (parent), GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);

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
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), rname);

	// Set default dialog button
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	// Run dialog window
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		// Get chosen file name
		gchar *path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		// Create error object
		GError *error = NULL;

		// Save report into file
		if (!SaveFunction (path, GTK_TREE_MODEL (model), &error))
			ShowFileErrorMessage (GTK_WINDOW (dialog), "Can not save report", error);
		else
		{
			// Extract file name from path
			gchar *fname = g_filename_display_basename (path);

			// Create string buffer
			GString *sstring = g_string_new (NULL);

			// Create status bar string
			g_string_printf (sstring, "Report saved into file '%s'", fname);

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

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handler for "Calcel" button                                    //
//****************************************************************************//
static void CancelButton (GtkButton *button, gpointer data)
{
	// Convert data pointer
	GtkWindow *window = reinterpret_cast <GtkWindow*> (data);

	// Close progress window
	gtk_window_close (GTK_WINDOW (window));
}

//****************************************************************************//
//      Signal handler for window close signal                                //
//****************************************************************************//
static gboolean WindowClose (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Convert data pointer
	gboolean *flag = reinterpret_cast <gboolean*> (data);

	// Set termination flag
	*flag = TRUE;

	// Return continue state
	return FALSE;
}

//****************************************************************************//
//      Progress dialog                                                       //
//****************************************************************************//
ProgressDialog CreateProgressDialog (GtkWindow *parent, const gchar *message, gboolean *flag)
{
	// Create dialog window
	GtkWidget *dialog = gtk_dialog_new_with_buttons ("Please wait", GTK_WINDOW (parent), GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL, NULL);

	// Get default dialog button
	GtkWidget *button = gtk_dialog_get_widget_for_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

	// Get content area of dialog
	GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

	// Get action area of dialog
	GtkWidget *action = gtk_dialog_get_action_area (GTK_DIALOG (dialog));

	// Create grid
	GtkWidget *grid = gtk_grid_new ();

	// Create image
	GtkWidget *image = gtk_image_new_from_icon_name ("system-run", GTK_ICON_SIZE_DIALOG);

	// Create label fields
	GtkWidget *comment = gtk_label_new (message);

	// Create progress bar
	GtkWidget *progress = gtk_progress_bar_new ();

	// Add image to grid
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (image), 0, 0, 1, 2);

	// Add label to grid
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (comment), 1, 0, 1, 1);

	// Add progress bar to grid
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (progress), 1, 1, 1, 1);

	// Add grid to dialog content area
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (grid), TRUE, TRUE, 0);

	// Set label properties
	gtk_label_set_selectable (GTK_LABEL (comment), FALSE);
	gtk_label_set_single_line_mode (GTK_LABEL (comment), TRUE);
	gtk_widget_set_halign (GTK_WIDGET (comment), GTK_ALIGN_START);

	// Set progress bar properties
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress), 0.0);
	gtk_progress_bar_set_inverted (GTK_PROGRESS_BAR (progress), FALSE);
	gtk_progress_bar_set_show_text (GTK_PROGRESS_BAR (progress), TRUE);
	gtk_widget_set_size_request (GTK_WIDGET (progress), 250, -1);

	// Set grid properties
	guint box_border = gtk_container_get_border_width (GTK_CONTAINER (box));
	guint action_border = gtk_container_get_border_width (GTK_CONTAINER (action));
	gtk_container_set_border_width (GTK_CONTAINER (grid), action_border);
	gtk_container_set_border_width (GTK_CONTAINER (grid), action_border);
	gtk_grid_set_column_spacing (GTK_GRID (grid), box_border + action_border);

	// Assign signal handlers
	g_signal_connect (G_OBJECT (dialog), "delete_event", G_CALLBACK (WindowClose), flag);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (CancelButton), dialog);

	// Show all window elements
	gtk_widget_show_all (GTK_WIDGET (dialog));

	// Return progress window structure
	return {GTK_WINDOW (dialog), GTK_PROGRESS_BAR (progress)};
}

//****************************************************************************//
//      Report dialog                                                         //
//****************************************************************************//
gboolean CreateReportDialog (GtkWindow *parent, const gchar *title, const gchar *rname, GtkTreeModel *stocks, GtkTreeModel *report, GList *good, GList *bad, GtkWidget* (*CreateList) (GtkTreeModel *model), gboolean (*SaveReport) (const gchar *fname, GtkTreeModel *model, GError **error), gint total, gint errors)
{
	// Operation status
	gboolean status = FALSE;

	// Create status lists
	StatusList glist = {stocks, good};
	StatusList blist = {stocks, bad};

	// Create dialog window
	GtkWidget *window = gtk_dialog_new_with_buttons (title, GTK_WINDOW (parent), GTK_DIALOG_MODAL, "_Close", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);

	// Get content area of dialog
	GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (window));

	// Get action area of dialog
	GtkWidget *action = gtk_dialog_get_action_area (GTK_DIALOG (window));

	// Add items to box
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateToolBar (&glist, &blist)), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateList (report)), TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateReportSummary (total, errors, gtk_container_get_border_width (GTK_CONTAINER (box)), gtk_container_get_border_width (GTK_CONTAINER (action)))), FALSE, FALSE, 0);

	// Set default dialog button
	gtk_dialog_set_default_response (GTK_DIALOG (window), GTK_RESPONSE_ACCEPT);

	// Show all box elements
	gtk_widget_show_all (GTK_WIDGET (box));

	// Run dialog window
	if (gtk_dialog_run (GTK_DIALOG (window)) == GTK_RESPONSE_ACCEPT)
	{
		// Set operation status
		status = SaveReportDialog (GTK_WINDOW (window), rname, GTK_TREE_MODEL (report), SaveReport);
	}

	// Clear list
	gtk_list_store_clear (GTK_LIST_STORE (report));

	// Decrement reference count to list
	g_object_unref (GTK_LIST_STORE (report));

	// Release lists of references
	g_list_free_full (good, reinterpret_cast <GDestroyNotify> (gtk_tree_row_reference_free));
	g_list_free_full (bad, reinterpret_cast <GDestroyNotify> (gtk_tree_row_reference_free));

	// Destroy dialog widget
	gtk_widget_destroy (GTK_WIDGET (window));

	// Return operation status
	return status;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
