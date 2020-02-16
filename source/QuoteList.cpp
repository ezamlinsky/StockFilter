/*                                                                 QuoteList.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                  QUOTE LIST                                  #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# include	<Common.h>
# include	<QuoteList.h>
# include	<Math.h>
# include	<Numbers.h>

//****************************************************************************//
//      Internal constants                                                    //
//****************************************************************************//

//============================================================================//
//      Quote price range                                                     //
//============================================================================//
# define	PRICE_DIGITS	2			// Amount of decimal digits price have
# define	PRICE_MIN		0.01		// Min quote price value
# define	PRICE_MAX		1000000		// Max quote price value
# define	PRICE_STEP		0.01		// Step increment for quote price change
# define	PRICE_PAGE		1.00		// Page increment for quote price change

//============================================================================//
//      Quote volume range                                                    //
//============================================================================//
# define	VOLUME_DIGITS	0			// Amount of decimal digits volume have
# define	VOLUME_MIN		0			// Min quote volume value
# define	VOLUME_MAX		1000000000	// Max quote volume value
# define	VOLUME_STEP		100			// Step increment for quote volume change
# define	VOLUME_PAGE		1000		// Page increment for quote volume change

//****************************************************************************//
//      Global variables                                                      //
//****************************************************************************//
extern	guint		fileid;

//****************************************************************************//
//      Local objects                                                         //
//****************************************************************************//
static	GtkWidget	*window;
static	GtkWidget	*treeview;

//****************************************************************************//
//      Extract date from string                                              //
//****************************************************************************//
time_t ExtractDate (const gchar *string, GError **error)
{
	// Current symbol position into string
	gsize len;

	// Extract year
	sint32_t year;
	len = Numbers::DecToNum (&year, string) + 1;
	string += len;
	if (len == 0)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Missing year value");
		return TIME_ERROR;
	}
	else if (len == 1)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Year value overflow");
		return TIME_ERROR;
	}

	// Extract month
	uint8_t mon;
	len = Numbers::DecToNum (&mon, string) + 1;
	string += len;
	if (len == 0)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Missing month value");
		return TIME_ERROR;
	}
	else if (len == 1)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Month value overflow");
		return TIME_ERROR;
	}

	// Extract day
	uint8_t day;
	len = Numbers::DecToNum (&day, string) + 1;
	string += len;
	if (len == 0)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Missing day value");
		return TIME_ERROR;
	}
	else if (len == 1)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Day value overflow");
		return TIME_ERROR;
	}

	// Convert date to unix time
	time_t date = Time::ConvertDate (day, mon, year, 0, 0, 0);
	if (date == static_cast <time_t> (TIME_ERROR))
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Incorrect time stamp");
		return TIME_ERROR;
	}

	// Return date value
	return date;
}

//****************************************************************************//
//      Extract price from string                                             //
//****************************************************************************//
gfloat ExtractPrice (const gchar *string, const gchar *field, GError **error)
{
	// Extract price
	gfloat price;
	gsize len = Numbers::DecToNum (&price, string) + 1;
	if (len == 0)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Missing %s price value", field);
		return M_NAN;
	}
	else if (len == 1)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Overflow occurred in %s price", field);
		return M_NAN;
	}

	// Check if price is finite numerical value
	if (!Math::IsFinite (price))
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Non finite %s price", field);
		return M_NAN;
	}
	else if (price <= 0)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Non positive %s price", field);
		return M_NAN;
	}

	// Return price value
	return price;
}

//****************************************************************************//
//      Extract volume from string                                            //
//****************************************************************************//
gsize ExtractVolume (const gchar *string, GError **error)
{
	// Extract volume
	gsize volume;
	gsize len = Numbers::DecToNum (&volume, string) + 1;
	if (len == 0)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Missing volume value");
		return -1;
	}
	else if (len == 1)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Volume value overflow");
		return -1;
	}

	// Return volume value
	return volume;
}

//****************************************************************************//
//      Check if quote correct                                                //
//****************************************************************************//
gboolean IsQuoteCorrect (time_t date, gfloat open, gfloat high, gfloat low, gfloat close, GError **error)
{
	// Check low price
	if (low <= 0)
	{
		// Set error message
		date_struct curdate = Time::ExtractDate (date);
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "%.4i-%.2d-%.2d: Incorrect low price %.2f", curdate.year, curdate.mon, curdate.day, low);
		return FALSE;
	}

	// Check open and low prices
	if (open < low)
	{
		// Set error message
		date_struct curdate = Time::ExtractDate (date);
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "%.4i-%.2d-%.2d: Open price %.2f < low price %.2f", curdate.year, curdate.mon, curdate.day, open, low);
		return FALSE;
	}

	// Check open and high prices
	if (open > high)
	{
		// Set error message
		date_struct curdate = Time::ExtractDate (date);
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "%.4i-%.2d-%.2d: Open price %.2f > high price %.2f", curdate.year, curdate.mon, curdate.day, open, high);
		return FALSE;
	}

	// Check close and low prices
	if (close < low)
	{
		// Set error message
		date_struct curdate = Time::ExtractDate (date);
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "%.4i-%.2d-%.2d: Close price %.2f < low price %.2f", curdate.year, curdate.mon, curdate.day, close, low);
		return FALSE;
	}

	// Check close and high prices
	if (close > high)
	{
		// Set error message
		date_struct curdate = Time::ExtractDate (date);
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "%.4i-%.2d-%.2d: Close price %.2f > high price %.2f", curdate.year, curdate.mon, curdate.day, close, high);
		return FALSE;
	}

	// Normal exit
	return TRUE;
}

//****************************************************************************//
//      Open quote list                                                       //
//****************************************************************************//
gboolean OpenQuoteList (Quotes *quotes, const gchar *fname, GError **error)
{
	// Check if quote file exists
	if (g_file_test (fname, G_FILE_TEST_IS_REGULAR))
	{
		// Open existing quote file
		return quotes -> OpenList (fname, error);
	}
	else
	{
		// Create new quote file
		quotes -> NewList (MIN_DATE);
		return TRUE;
	}
}

//****************************************************************************//
//      Save quote list                                                       //
//****************************************************************************//
gboolean SaveQuoteList (Quotes *quotes, const gchar *fname, GError **error)
{
	// Get quote directory
	gchar *temp = g_strdup (fname);
	*(g_utf8_strrchr (temp, -1, '/')) = '\0';

	// Make quote directory if does not exist
	g_mkdir_with_parents (temp, 0755);

	// Free temporary string buffer
	g_free (temp);

	// Save quote file
	return quotes -> SaveList (fname, error);
}

//****************************************************************************//
//      Check if date unique                                                  //
//****************************************************************************//
static gboolean IsDateUnique (GtkTreeModel *model, time_t value, GError **error)
{
	// Get iterator position
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	{
		// Iterate through all elements
		do {
			// Get date value
			time_t date;
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, QUOTE_DATE_ID, &date, -1);

			// If value is already into the list
			if (date == value)
			{
				// Set error message
				date_struct curdate = Time::ExtractDate (date);
				g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Quote '%.4i-%.2d-%.2d' already exists", curdate.year, curdate.mon, curdate.day);
				return FALSE;
			}

			// Change iterator position to next element
		} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
	}

	// Normal exit
	return TRUE;
}

//****************************************************************************//
//      Signal handler for "Import" tool button                               //
//****************************************************************************//
static void ImportQuotes (GtkToolButton *toolbutton, gpointer data)
{
	// Convert data pointer
	Quotes *quotes = reinterpret_cast <Quotes*> (data);

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Create dialog window
		GtkWidget *dialog = gtk_file_chooser_dialog_new ("Import quotes from...", GTK_WINDOW (window), GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Import", GTK_RESPONSE_ACCEPT, NULL);

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

			// Import quote list from file
			if (!quotes -> ImportList (path, &error))
				ShowFileErrorMessage (GTK_WINDOW (dialog), "Can not import quote list", error);
			else
			{
				// Extract file name from path
				gchar *fname = g_filename_display_basename (path);

				// Create string buffer
				GString *sstring = g_string_new (NULL);

				// Create status bar string
				g_string_printf (sstring, "Quotes imported from file '%s'", fname);

				// Show status message
				ShowStatusMessage (sstring -> str, fileid);

				// Free temporary string buffer
				g_free (fname);

				// Relase string buffer
				g_string_free (sstring, TRUE);

				// Set tree view model
				gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (quotes -> GetQuotes ()));
			}

			// Free temporary string buffer
			g_free (path);
		}

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}
}

//****************************************************************************//
//      Signal handler for "Export" tool button                               //
//****************************************************************************//
static void ExportQuotes (GtkToolButton *toolbutton, gpointer data)
{
	// Convert data pointer
	Quotes *quotes = reinterpret_cast <Quotes*> (data);

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Create dialog window
		GtkWidget *dialog = gtk_file_chooser_dialog_new ("Export quotes to...", GTK_WINDOW (window), GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL, "_Export", GTK_RESPONSE_ACCEPT, NULL);

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
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "Quotes.tsv");

		// Set default dialog button
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

		// Run dialog window
		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			// Get chosen file name
			gchar *path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

			// Create error object
			GError *error = NULL;

			// Try to set quote list
			if (!quotes -> SetQuotes (GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview))), &error))
				ShowFileErrorMessage (GTK_WINDOW (window), "Found errors into quote list", error);
			else
			{
				// Export quote list into file
				if (!quotes -> ExportList (path, &error))
					ShowFileErrorMessage (GTK_WINDOW (dialog), "Can not export quote list", error);
				else
				{
					// Extract file name from path
					gchar *fname = g_filename_display_basename (path);

					// Create string buffer
					GString *sstring = g_string_new (NULL);

					// Create status bar string
					g_string_printf (sstring, "Quotes exported into file '%s'", fname);

					// Show status message
					ShowStatusMessage (sstring -> str, fileid);

					// Free temporary string buffer
					g_free (fname);

					// Relase string buffer
					g_string_free (sstring, TRUE);
				}
			}

			// Free temporary string buffer
			g_free (path);
		}

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}
}

//****************************************************************************//
//      Signal handler for "Insert" tool button                               //
//****************************************************************************//
static void InsertQuotes (GtkToolButton *toolbutton, gpointer data)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Create dialog window
		GtkWidget *dialog = gtk_dialog_new_with_buttons ("New stock quote", GTK_WINDOW (window), GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL, "_Insert", GTK_RESPONSE_ACCEPT, NULL);

		// Get content area of dialog
		GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

		// Get action area of dialog
		GtkWidget *action = gtk_dialog_get_action_area (GTK_DIALOG (dialog));

		// Create alignment
		GtkWidget *alignment = gtk_alignment_new (0, 0, 1, 1);

		// Create grid
		GtkWidget *grid = gtk_grid_new ();

		// Create label fields
		GtkWidget *DateLabel = gtk_label_new (QUOTE_DATE_LABEL);
		GtkWidget *OpenLabel = gtk_label_new (QUOTE_OPEN_LABEL);
		GtkWidget *HighLabel = gtk_label_new (QUOTE_HIGH_LABEL);
		GtkWidget *LowLabel = gtk_label_new (QUOTE_LOW_LABEL);
		GtkWidget *CloseLabel = gtk_label_new (QUOTE_CLOSE_LABEL);
		GtkWidget *AdjCloseLabel = gtk_label_new (QUOTE_ADJCLOSE_LABEL);
		GtkWidget *VolumeLabel = gtk_label_new (QUOTE_VOLUME_LABEL);

		// Create text entry fields
		GtkWidget *DateEntry = gtk_entry_new ();

		// Create spin buttons
		GtkWidget *OpenSpin = gtk_spin_button_new_with_range (PRICE_MIN, PRICE_MAX, PRICE_STEP);
		GtkWidget *HighSpin = gtk_spin_button_new_with_range (PRICE_MIN, PRICE_MAX, PRICE_STEP);
		GtkWidget *LowSpin = gtk_spin_button_new_with_range (PRICE_MIN, PRICE_MAX, PRICE_STEP);
		GtkWidget *CloseSpin = gtk_spin_button_new_with_range (PRICE_MIN, PRICE_MAX, PRICE_STEP);
		GtkWidget *AdjCloseSpin = gtk_spin_button_new_with_range (PRICE_MIN, PRICE_MAX, PRICE_STEP);
		GtkWidget *VolumeSpin = gtk_spin_button_new_with_range (VOLUME_MIN, VOLUME_MAX, VOLUME_STEP);

		// Add labels to grid
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (DateLabel), 0, 0, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (OpenLabel), 0, 1, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (HighLabel), 0, 2, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (LowLabel), 0, 3, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (CloseLabel), 0, 4, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (AdjCloseLabel), 0, 5, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (VolumeLabel), 0, 6, 1, 1);

		// Add spin buttons to grid
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (DateEntry), 1, 0, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (OpenSpin), 1, 1, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (HighSpin), 1, 2, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (LowSpin), 1, 3, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (CloseSpin), 1, 4, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (AdjCloseSpin), 1, 5, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (VolumeSpin), 1, 6, 1, 1);

		// Add tooltip text to spin buttons
		gtk_widget_set_tooltip_text (GTK_WIDGET (DateEntry), "Quote date");
		gtk_widget_set_tooltip_text (GTK_WIDGET (OpenSpin), "Quote open price");
		gtk_widget_set_tooltip_text (GTK_WIDGET (HighSpin), "Quote high price");
		gtk_widget_set_tooltip_text (GTK_WIDGET (LowSpin), "Quote low price");
		gtk_widget_set_tooltip_text (GTK_WIDGET (CloseSpin), "Quote close price");
		gtk_widget_set_tooltip_text (GTK_WIDGET (AdjCloseSpin), "Quote adjusted close price");
		gtk_widget_set_tooltip_text (GTK_WIDGET (VolumeSpin), "Amount of stocks were traded");

		// Add grid to alignment
		gtk_container_add (GTK_CONTAINER (alignment), grid);

		// Add alignment to dialog content area
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (alignment), TRUE, TRUE, 0);

		// Set label properties
		gtk_label_set_selectable (GTK_LABEL (DateLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (OpenLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (HighLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (LowLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (CloseLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (AdjCloseLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (VolumeLabel), FALSE);
		gtk_label_set_single_line_mode (GTK_LABEL (DateLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (OpenLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (HighLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (LowLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (CloseLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (AdjCloseLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (VolumeLabel), TRUE);
		gtk_widget_set_halign (GTK_WIDGET (DateLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (OpenLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (HighLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (LowLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (CloseLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (AdjCloseLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (VolumeLabel), GTK_ALIGN_END);

		// Set text entry properties
		gtk_entry_set_placeholder_text (GTK_ENTRY (DateEntry), "YYYY-MM-DD");
		gtk_widget_set_hexpand (GTK_WIDGET (DateEntry), TRUE);

		// Set spin button properties
		gtk_spin_button_set_digits (GTK_SPIN_BUTTON (OpenSpin), PRICE_DIGITS);
		gtk_spin_button_set_digits (GTK_SPIN_BUTTON (HighSpin), PRICE_DIGITS);
		gtk_spin_button_set_digits (GTK_SPIN_BUTTON (LowSpin), PRICE_DIGITS);
		gtk_spin_button_set_digits (GTK_SPIN_BUTTON (CloseSpin), PRICE_DIGITS);
		gtk_spin_button_set_digits (GTK_SPIN_BUTTON (AdjCloseSpin), PRICE_DIGITS);
		gtk_spin_button_set_digits (GTK_SPIN_BUTTON (VolumeSpin), VOLUME_DIGITS);
		gtk_spin_button_set_increments (GTK_SPIN_BUTTON (OpenSpin), PRICE_STEP, PRICE_PAGE);
		gtk_spin_button_set_increments (GTK_SPIN_BUTTON (HighSpin), PRICE_STEP, PRICE_PAGE);
		gtk_spin_button_set_increments (GTK_SPIN_BUTTON (LowSpin), PRICE_STEP, PRICE_PAGE);
		gtk_spin_button_set_increments (GTK_SPIN_BUTTON (CloseSpin), PRICE_STEP, PRICE_PAGE);
		gtk_spin_button_set_increments (GTK_SPIN_BUTTON (AdjCloseSpin), PRICE_STEP, PRICE_PAGE);
		gtk_spin_button_set_increments (GTK_SPIN_BUTTON (VolumeSpin), VOLUME_STEP, VOLUME_PAGE);
		gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (OpenSpin), GTK_UPDATE_IF_VALID);
		gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (HighSpin), GTK_UPDATE_IF_VALID);
		gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (LowSpin), GTK_UPDATE_IF_VALID);
		gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (CloseSpin), GTK_UPDATE_IF_VALID);
		gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (AdjCloseSpin), GTK_UPDATE_IF_VALID);
		gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (VolumeSpin), GTK_UPDATE_IF_VALID);
		gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (OpenSpin), TRUE);
		gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (HighSpin), TRUE);
		gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (LowSpin), TRUE);
		gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (CloseSpin), TRUE);
		gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (AdjCloseSpin), TRUE);
		gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (VolumeSpin), TRUE);
		gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (OpenSpin), TRUE);
		gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (HighSpin), TRUE);
		gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (LowSpin), TRUE);
		gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (CloseSpin), TRUE);
		gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (AdjCloseSpin), TRUE);
		gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (VolumeSpin), TRUE);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (OpenSpin), FALSE);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (HighSpin), FALSE);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (LowSpin), FALSE);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (CloseSpin), FALSE);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (AdjCloseSpin), FALSE);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (VolumeSpin), FALSE);
		gtk_widget_set_hexpand (GTK_WIDGET (OpenSpin), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (HighSpin), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (LowSpin), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (CloseSpin), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (AdjCloseSpin), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (VolumeSpin), TRUE);

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
			// Extract quote information from dialog inputs
			const gchar *datestr = gtk_entry_get_text (GTK_ENTRY (DateEntry));
			gfloat open = gtk_spin_button_get_value (GTK_SPIN_BUTTON (OpenSpin));
			gfloat high = gtk_spin_button_get_value (GTK_SPIN_BUTTON (HighSpin));
			gfloat low = gtk_spin_button_get_value (GTK_SPIN_BUTTON (LowSpin));
			gfloat close = gtk_spin_button_get_value (GTK_SPIN_BUTTON (CloseSpin));
			gfloat adjclose = gtk_spin_button_get_value (GTK_SPIN_BUTTON (AdjCloseSpin));
			gsize volume = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (VolumeSpin));

			// Create error object
			GError *error = NULL;

			// Extract date from string
			time_t date = ExtractDate (datestr, &error);

			// Check if date is correct
			if (date == static_cast <time_t> (TIME_ERROR))
				ShowErrorMessage (GTK_WINDOW (dialog), "Incorrect quote date", error);
			else
			{
				// Check if new date unique
				if (!IsDateUnique (GTK_TREE_MODEL (model), date, &error))
					ShowErrorMessage (GTK_WINDOW (window), "Duplicate dates are not allowed", error);
				else
				{
					// Check if quote correct
					if (!IsQuoteCorrect (date, open, high, low, close, &error))
						ShowErrorMessage (GTK_WINDOW (window), "Incorrect stock quote", error);
					else
					{
						// Add new element to list store object
						GtkTreeIter iter;
						gtk_list_store_append (GTK_LIST_STORE (model), &iter);
						gtk_list_store_set (GTK_LIST_STORE (model), &iter, QUOTE_DATE_ID, date, QUOTE_OPEN_ID, open, QUOTE_HIGH_ID, high, QUOTE_LOW_ID, low, QUOTE_CLOSE_ID, close, QUOTE_ADJCLOSE_ID, adjclose, QUOTE_VOLUME_ID, volume, -1);
					}
				}
			}
		}

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}
}

//****************************************************************************//
//      Signal handler for "Remove" tool button                               //
//****************************************************************************//
static void RemoveQuotes (GtkToolButton *toolbutton, gpointer data)
{
	// Get tree view selection
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

	// Get selected rows
	GtkTreeModel *model;
	GList *plist = gtk_tree_selection_get_selected_rows (GTK_TREE_SELECTION (selection), &model);
	GList *rlist = NULL;
	GList *element = NULL;

	// Process all path list elements
	element = g_list_first (plist);
	while (element)
	{
		// Append new element to reference list
		rlist = g_list_append (rlist, gtk_tree_row_reference_new (GTK_TREE_MODEL (model), reinterpret_cast <GtkTreePath*> (element -> data)));

		// Go to next path list element
		element = element -> next;
	}

	// Process all reference list elements
	element = g_list_first (rlist);
	while (element)
	{
		// Get tree path
		GtkTreePath *path = gtk_tree_row_reference_get_path (reinterpret_cast <GtkTreeRowReference*> (element -> data));
		if (path)
		{
			// Get iterator position
			GtkTreeIter iter;
			if (gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path))
			{
				// Remove quote from the list
				gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
			}
		}

		// Go to next reference list element
		element = element -> next;
	}

	// Release list of selected rows
	g_list_free_full (plist, reinterpret_cast <GDestroyNotify> (gtk_tree_path_free));

	// Release list of references
	g_list_free_full (rlist, reinterpret_cast <GDestroyNotify> (gtk_tree_row_reference_free));
}

//****************************************************************************//
//      Signal handler for date cell                                          //
//****************************************************************************//
static void DateCellHandler (GtkCellRendererText *cell, gchar *path, gchar *newval, gpointer data)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path))
		{
			// Get old date value
			time_t odate;
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, GPOINTER_TO_UINT (data), &odate, -1);

			// Create error object
			GError *error = NULL;

			// Extract new date from string
			time_t ndate = ExtractDate (newval, &error);

			// Check if new date is correct
			if (ndate == static_cast <time_t> (TIME_ERROR))
				ShowErrorMessage (GTK_WINDOW (window), "Incorrect quote date", error);
			else
			{
				// Check if date is changed)
				if (odate != ndate)
				{
					// Check if new date unique
					if (!IsDateUnique (GTK_TREE_MODEL (model), ndate, &error))
						ShowErrorMessage (GTK_WINDOW (window), "Duplicate dates are not allowed", error);
					else
					{
						// Set new date
						gtk_list_store_set (GTK_LIST_STORE (model), &iter, GPOINTER_TO_UINT (data), ndate, -1);
					}
				}
			}
		}
	}
}

//****************************************************************************//
//      Signal handler for price cells                                        //
//****************************************************************************//
static void PriceCellHandler (GtkCellRendererText *cell, gchar *path, gchar *newval, gpointer data)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path))
		{
			// Get old quote
			time_t date;
			gfloat open, high, low, close, adjclose;
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, QUOTE_DATE_ID, &date, QUOTE_OPEN_ID, &open, QUOTE_HIGH_ID, &high, QUOTE_LOW_ID, &low, QUOTE_CLOSE_ID, &close, QUOTE_ADJCLOSE_ID, &adjclose, -1);

			// Create error object
			GError *error = NULL;

			// Extract price from string
			gfloat price = M_NAN;
			switch (GPOINTER_TO_UINT (data))
			{
				case QUOTE_OPEN_ID:
					price = ExtractPrice (newval, "open", &error);
					open = price;
					break;
				case QUOTE_HIGH_ID:
					price = ExtractPrice (newval, "high", &error);
					high = price;
					break;
				case QUOTE_LOW_ID:
					price = ExtractPrice (newval, "low", &error);
					low = price;
					break;
				case QUOTE_CLOSE_ID:
					price = ExtractPrice (newval, "close", &error);
					close = price;
					break;
				case QUOTE_ADJCLOSE_ID:
					price = ExtractPrice (newval, "adjusted closse", &error);
					adjclose = price;
					break;
			}

			// Check if price is correct
			if (Math::IsNaN (price))
				ShowErrorMessage (GTK_WINDOW (window), "Incorrect quote price", error);
			else
			{
				// Check if quote correct
				if (!IsQuoteCorrect (date, open, high, low, close, &error))
					ShowErrorMessage (GTK_WINDOW (window), "Incorrect stock quote", error);
				else
				{
					// Set quote
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, QUOTE_DATE_ID, date, QUOTE_OPEN_ID, open, QUOTE_HIGH_ID, high, QUOTE_LOW_ID, low, QUOTE_CLOSE_ID, close, QUOTE_ADJCLOSE_ID, adjclose, -1);
				}
			}
		}
	}
}

//****************************************************************************//
//      Signal handler for volume cell                                        //
//****************************************************************************//
static void VolumeCellHandler (GtkCellRendererText *cell, gchar *path, gchar *newval, gpointer data)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path))
		{
			// Create error object
			GError *error = NULL;

			// Extract volume from string
			gsize nvolume = ExtractVolume (newval, &error);

			// Check if volume is correct
			if (nvolume == static_cast <gsize> (-1))
				ShowErrorMessage (GTK_WINDOW (window), "Incorrect quote volume", error);
			else
			{
				// Set volume
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, GPOINTER_TO_UINT (data), nvolume, -1);
			}
		}
	}
}

//****************************************************************************//
//      Tool bar                                                              //
//****************************************************************************//
static GtkWidget* CreateToolBar (Quotes *quotes)
{
	// Create tool bar
	GtkWidget *toolbar = gtk_toolbar_new ();

	// Create buttons
	GtkToolItem *Import = gtk_tool_button_new (NULL, "Import");
	GtkToolItem *Export = gtk_tool_button_new (NULL, "Export");
	GtkToolItem *Separator = gtk_separator_tool_item_new();
	GtkToolItem *Insert = gtk_tool_button_new (NULL, "Insert");
	GtkToolItem *Remove = gtk_tool_button_new (NULL, "Remove");

	// Add buttons to tool bar
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Import), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Export), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Separator), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Insert), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Remove), -1);

	// Add tooltip text to buttons
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Import), "Import quotes from TSV file");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Export), "Export quotes into TSV file");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Insert), "Insert new quote into quote list");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Remove), "Remove selected quotes from quote list");
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (Import), TRUE);
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (Export), TRUE);
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (Insert), TRUE);
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (Remove), TRUE);

	// Set button properties
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Import), "document-import");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Export), "document-export");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Insert), "list-add");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Remove), "list-remove");

	// Set tool bar properties
	gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), TRUE);
	gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH_HORIZ);
	gtk_toolbar_set_icon_size (GTK_TOOLBAR (toolbar), GTK_ICON_SIZE_LARGE_TOOLBAR);

	// Assign signal handlers
	g_signal_connect (G_OBJECT (Import), "clicked", G_CALLBACK (ImportQuotes), quotes);
	g_signal_connect (G_OBJECT (Export), "clicked", G_CALLBACK (ExportQuotes), quotes);
	g_signal_connect (G_OBJECT (Insert), "clicked", G_CALLBACK (InsertQuotes), NULL);
	g_signal_connect (G_OBJECT (Remove), "clicked", G_CALLBACK (RemoveQuotes), NULL);

	// Return tool bar
	return toolbar;
}

//****************************************************************************//
//      Quote list                                                            //
//****************************************************************************//
static GtkWidget* CreateQuoteList (Quotes *quotes)
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
	GtkCellRenderer *DateCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *OpenCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *HighCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *LowCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *CloseCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *AdjCloseCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *VolumeCell = gtk_cell_renderer_text_new ();

	// Create columns for tree view
	GtkTreeViewColumn *DateColumn = gtk_tree_view_column_new_with_attributes (QUOTE_DATE_LABEL, GTK_CELL_RENDERER (DateCell), "text", QUOTE_DATE_ID, NULL);
	GtkTreeViewColumn *OpenColumn = gtk_tree_view_column_new_with_attributes (QUOTE_OPEN_LABEL, GTK_CELL_RENDERER (OpenCell), "text", QUOTE_OPEN_ID, NULL);
	GtkTreeViewColumn *HighColumn = gtk_tree_view_column_new_with_attributes (QUOTE_HIGH_LABEL, GTK_CELL_RENDERER (HighCell), "text", QUOTE_HIGH_ID, NULL);
	GtkTreeViewColumn *LowColumn = gtk_tree_view_column_new_with_attributes (QUOTE_LOW_LABEL, GTK_CELL_RENDERER (LowCell), "text", QUOTE_LOW_ID, NULL);
	GtkTreeViewColumn *CloseColumn = gtk_tree_view_column_new_with_attributes (QUOTE_CLOSE_LABEL, GTK_CELL_RENDERER (CloseCell), "text", QUOTE_CLOSE_ID, NULL);
	GtkTreeViewColumn *AdjCloseColumn = gtk_tree_view_column_new_with_attributes (QUOTE_ADJCLOSE_LABEL, GTK_CELL_RENDERER (AdjCloseCell), "text", QUOTE_ADJCLOSE_ID, NULL);
	GtkTreeViewColumn *VolumeColumn = gtk_tree_view_column_new_with_attributes (QUOTE_VOLUME_LABEL, GTK_CELL_RENDERER (VolumeCell), "text", QUOTE_VOLUME_ID, NULL);

	// Add columns to tree view
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (DateColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (OpenColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (HighColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (LowColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (CloseColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (AdjCloseColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (VolumeColumn));

	// Add tree view to scrolled window
	gtk_container_add (GTK_CONTAINER (scrolled), GTK_WIDGET (treeview));

	// Set cell properties
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (DateCell), 0.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (OpenCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (HighCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (LowCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (CloseCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (AdjCloseCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (VolumeCell), 1.0, 0.0);
	g_object_set (G_OBJECT (DateCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (OpenCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (HighCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (LowCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (CloseCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (AdjCloseCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (VolumeCell), "editable", TRUE, NULL);

	// Set column properties
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (DateColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (OpenColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (HighColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (LowColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (CloseColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (AdjCloseColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (VolumeColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (DateColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (OpenColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (HighColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (LowColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (CloseColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (AdjCloseColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (VolumeColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (DateColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (OpenColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (HighColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (LowColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (CloseColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (AdjCloseColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (VolumeColumn), TRUE);

	// Set tree view properties
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), FALSE);
	gtk_tree_view_set_rubber_banding (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_GRID_LINES_NONE);

	// Set scrolled window preperties
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_placement (GTK_SCROLLED_WINDOW (scrolled), GTK_CORNER_TOP_LEFT);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled), GTK_SHADOW_IN);
	gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (scrolled), 800);
	gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (scrolled), 600);

	// Set selection properties
	gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_MULTIPLE);

	// Assign cell data functions
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (DateColumn), GTK_CELL_RENDERER (DateCell), DateRenderFunc, GUINT_TO_POINTER (QUOTE_DATE_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (OpenColumn), GTK_CELL_RENDERER (OpenCell), PriceRenderFunc, GUINT_TO_POINTER (QUOTE_OPEN_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (HighColumn), GTK_CELL_RENDERER (HighCell), PriceRenderFunc, GUINT_TO_POINTER (QUOTE_HIGH_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (LowColumn), GTK_CELL_RENDERER (LowCell), PriceRenderFunc, GUINT_TO_POINTER (QUOTE_LOW_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (CloseColumn), GTK_CELL_RENDERER (CloseCell), PriceRenderFunc, GUINT_TO_POINTER (QUOTE_CLOSE_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (AdjCloseColumn), GTK_CELL_RENDERER (AdjCloseCell), PriceRenderFunc, GUINT_TO_POINTER (QUOTE_ADJCLOSE_ID), NULL);

	// Assign signal handlers for cells
	g_signal_connect (G_OBJECT (DateCell), "edited", G_CALLBACK (DateCellHandler), GUINT_TO_POINTER (QUOTE_DATE_ID));
	g_signal_connect (G_OBJECT (OpenCell), "edited", G_CALLBACK (PriceCellHandler), GUINT_TO_POINTER (QUOTE_OPEN_ID));
	g_signal_connect (G_OBJECT (HighCell), "edited", G_CALLBACK (PriceCellHandler), GUINT_TO_POINTER (QUOTE_HIGH_ID));
	g_signal_connect (G_OBJECT (LowCell), "edited", G_CALLBACK (PriceCellHandler), GUINT_TO_POINTER (QUOTE_LOW_ID));
	g_signal_connect (G_OBJECT (CloseCell), "edited", G_CALLBACK (PriceCellHandler), GUINT_TO_POINTER (QUOTE_CLOSE_ID));
	g_signal_connect (G_OBJECT (AdjCloseCell), "edited", G_CALLBACK (PriceCellHandler), GUINT_TO_POINTER (QUOTE_ADJCLOSE_ID));
	g_signal_connect (G_OBJECT (VolumeCell), "edited", G_CALLBACK (VolumeCellHandler), GUINT_TO_POINTER (QUOTE_VOLUME_ID));

	// Set tree view model
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (quotes -> GetQuotes ()));

	// Return scrolled window object
	return scrolled;
}

//****************************************************************************//
//      Edit quotes dialog                                                    //
//****************************************************************************//
gboolean EditQuotesDialog (GtkWindow *parent, const gchar *path, const gchar *ticker, const gchar *name, const gchar *country, const gchar *sector, const gchar *industry, const gchar *url)
{
	// Operation status
	gboolean status = FALSE;

	// Create quotes object
	Quotes quotes;

	// Create error object
	GError *error = NULL;

	// Try to open quotes
	if (!OpenQuoteList (&quotes, path, &error))
		ShowFileErrorMessage (GTK_WINDOW (parent), "Can not open stock quotes", error);
	else
	{
		// Extract file name from path
		gchar *fname = g_filename_display_basename (path);

		// Create string buffer
		GString *string = g_string_new (NULL);

		// Create title bar string
		g_string_printf (string, "Edit stock quotes - %s", fname);

		// Create dialog window
		window = gtk_dialog_new_with_buttons (string -> str, GTK_WINDOW (parent), GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);

		// Get content area of dialog
		GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (window));

		// Get action area of dialog
		GtkWidget *action = gtk_dialog_get_action_area (GTK_DIALOG (window));

		// Add items to box
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateToolBar (&quotes)), FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateQuoteList (&quotes)), TRUE, TRUE, 0);
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateStockSummary (ticker, name, country, sector, industry, url, gtk_container_get_border_width (GTK_CONTAINER (box)), gtk_container_get_border_width (GTK_CONTAINER (action)))), FALSE, FALSE, 0);

		// Set default dialog button
		gtk_dialog_set_default_response (GTK_DIALOG (window), GTK_RESPONSE_ACCEPT);

		// Show all box elements
		gtk_widget_show_all (GTK_WIDGET (box));

		// Run dialog window
		if (gtk_dialog_run (GTK_DIALOG (window)) == GTK_RESPONSE_ACCEPT)
		{
			// Try to set quote list
			if (!quotes.SetQuotes (GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview))), &error))
				ShowFileErrorMessage (GTK_WINDOW (window), "Found errors into quote list", error);
			else
			{
				// Try to save quotes file
				if (!SaveQuoteList (&quotes, path, &error))
					ShowFileErrorMessage (GTK_WINDOW (window), "Can not save stock quotes", error);
				else
				{
					// Decrement reference count to quote list
					g_object_unref (GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview))));

					// Set success state
					status = TRUE;
				}
			}
		}

		// Free temporary string buffer
		g_free (fname);

		// Relase string buffer
		g_string_free (string, TRUE);

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (window));
	}

	// Return operation status
	return status;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
