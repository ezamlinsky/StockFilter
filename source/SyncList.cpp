/*                                                                  SyncList.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                  SYNC LIST                                   #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# include	<Common.h>
# include	<Quotes.h>
# include	<Client.h>
# include	<TimeZone.h>
# include	<StockList.h>
# include	<QuoteList.h>
# include	<SyncList.h>

//****************************************************************************//
//      Sync result structure                                                 //
//****************************************************************************//
struct SyncResult
{
	gboolean	status;			// Sync status
	gint		count;			// Quotes count
	time_t		start;			// Start quote date
	time_t		end;			// End quote date
};

//****************************************************************************//
//      Sync stock quotes with quote server                                   //
//****************************************************************************//
SyncResult SyncQuotes (const gchar *fname, const gchar* ticker, TimeZone *timezone, Client *client, GError **error)
{
	// Init result structure
	SyncResult result = {
		static_cast <gboolean> (FALSE),
		static_cast <gint> (-1),
		static_cast <time_t> (TIME_ERROR),
		static_cast <time_t> (TIME_ERROR)
	};

	// Get quotes file name
	gchar* path = GetQuotesFile (fname, ticker);

	// Create quotes object
	Quotes quotes;

	// Try to open quotes
	if (OpenQuoteList (&quotes, path, error))
	{
		// Get last quote date
		time_t last = quotes.GetLastDate ();
		if (last == static_cast <time_t> (TIME_ERROR))
			last = MIN_DATE;
		else
			last += TIME_DAY;

		// Get current time in time zone
		time_t curr = timezone -> GetCurrentTime ();

		// Check quotes for splits and dividends
		if (client -> CheckSplits (ticker, last, curr, error))
		{
			// Clear stock quotes
			quotes.NewList (curr);

			// Set last quote date min date
			last = static_cast <time_t> (MIN_DATE);
		}

		// Get new quotes
		if (client -> GetQuotes (ticker, last, curr, error))
		{
			// Add new quotes
			if (quotes.AddQuotes (client -> GetQuoteList (), curr, error))
			{
				// Try to save quotes
				if (SaveQuoteList (&quotes, path, error))
				{
					// Set result structure fields
					result.status = TRUE;
					result.count = client -> GetCount ();
					result.start = client -> GetFirstDate ();
					result.end = client -> GetLastDate ();
				}
			}
		}
	}

	// Free temporary string buffer
	g_free (path);

	// Normal exit
	return result;
}

//****************************************************************************//
//      Save sync report function                                             //
//****************************************************************************//
static gboolean SaveSyncReport (const gchar *fname, GtkTreeModel *model, GError **error)
{
	// Create string buffer
	GString *string = g_string_new (NULL);

	// Check if report is set
	if (model)
	{
		// Append report header
		g_string_append_printf (string, SYNC_TICKER_LABEL "\t" SYNC_QUOTES_LABEL "\t" SYNC_START_LABEL "\t" SYNC_END_LABEL "\t" SYNC_STATUS_LABEL "\n");

		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
		{
			// Iterate through all elements
			do {
				// Get record details
				gchar *ticker, *status;
				gint count;
				time_t start, end;
				gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, SYNC_TICKER_ID, &ticker, SYNC_QUOTES_ID, &count, SYNC_START_ID, &start, SYNC_END_ID, &end, SYNC_STATUS_ID, &status, -1);

				// Append quote information into string buffer
				date_struct sdate = Time::ExtractDate (start);
				date_struct edate = Time::ExtractDate (end);
				g_string_append_printf (string, "%s\t%i\t%.4i-%.2d-%.2d\t%.4i-%.2d-%.2d\t%s\n", ticker, count, sdate.year, sdate.mon, sdate.day, edate.year, edate.mon, edate.day, status);

				// Change iterator position to next element
			} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
		}
	}

	// Try to save string buffer into file
	gboolean status = g_file_set_contents (fname, string -> str, string -> len, error);

	// Relase string buffer
	g_string_free (string, TRUE);

	// Return file operation status
	return status;
}

//****************************************************************************//
//      Sync list                                                             //
//****************************************************************************//
static GtkWidget* CreateSyncList (GtkTreeModel *model)
{
	// Create tree view
	GtkWidget *treeview = gtk_tree_view_new ();

	// Get tree view adjustments
	GtkAdjustment *vadjustment = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (treeview));
	GtkAdjustment *hadjustment = gtk_scrollable_get_hadjustment (GTK_SCROLLABLE (treeview));

	// Create scrolled window
	GtkWidget *scrolled = gtk_scrolled_window_new (hadjustment, vadjustment);

	// Create cell renderers for columns
	GtkCellRenderer *TickerCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *QuotesCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *StartCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *EndCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *StatusCell = gtk_cell_renderer_text_new ();

	// Create columns for tree view
	GtkTreeViewColumn *TickerColumn = gtk_tree_view_column_new_with_attributes (SYNC_TICKER_LABEL, GTK_CELL_RENDERER (TickerCell), "text", SYNC_TICKER_ID, NULL);
	GtkTreeViewColumn *QuotesColumn = gtk_tree_view_column_new_with_attributes (SYNC_QUOTES_LABEL, GTK_CELL_RENDERER (QuotesCell), "text", SYNC_QUOTES_ID, NULL);
	GtkTreeViewColumn *StartColumn = gtk_tree_view_column_new_with_attributes (SYNC_START_LABEL, GTK_CELL_RENDERER (StartCell), "text", SYNC_START_ID, NULL);
	GtkTreeViewColumn *EndColumn = gtk_tree_view_column_new_with_attributes (SYNC_END_LABEL, GTK_CELL_RENDERER (EndCell), "text", SYNC_END_ID, NULL);
	GtkTreeViewColumn *StatusColumn = gtk_tree_view_column_new_with_attributes (SYNC_STATUS_LABEL, GTK_CELL_RENDERER (StatusCell), "text", SYNC_STATUS_ID, NULL);

	// Add columns to tree view
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (TickerColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (QuotesColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (StartColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (EndColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (StatusColumn));

	// Add tree view to scrolled window
	gtk_container_add (GTK_CONTAINER (scrolled), GTK_WIDGET (treeview));

	// Set cell properties
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (TickerCell), 0.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (QuotesCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (StartCell), 0.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (EndCell), 0.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (StatusCell), 0.0, 0.0);

	// Set column properties
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (TickerColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (QuotesColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (StartColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (EndColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (StatusColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (TickerColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (QuotesColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (StartColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (EndColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (StatusColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (TickerColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (QuotesColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (StartColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (EndColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (StatusColumn), TRUE);

	// Set tree view sort properties
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (TickerColumn), SYNC_TICKER_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (QuotesColumn), SYNC_QUOTES_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (StartColumn), SYNC_START_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (EndColumn), SYNC_END_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (StatusColumn), SYNC_STATUS_ID);

	// Set tree view properties
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_GRID_LINES_NONE);

	// Set scrolled window preperties
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_placement (GTK_SCROLLED_WINDOW (scrolled), GTK_CORNER_TOP_LEFT);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled), GTK_SHADOW_IN);
	gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (scrolled), 1000);
	gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (scrolled), 600);

	// Assign cell data functions
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (QuotesColumn), GTK_CELL_RENDERER (QuotesCell), CountRenderFunc, GUINT_TO_POINTER (SYNC_QUOTES_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (StartColumn), GTK_CELL_RENDERER (StartCell), DateRenderFunc, GUINT_TO_POINTER (SYNC_START_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (EndColumn), GTK_CELL_RENDERER (EndCell), DateRenderFunc, GUINT_TO_POINTER (SYNC_END_ID), NULL);

	// Set search column for immediate search
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), SYNC_TICKER_ID);

	// Set tree view model
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (model));

	// Return scrolled window object
	return scrolled;
}

//****************************************************************************//
//      Sync quotes dialog                                                    //
//****************************************************************************//
gboolean SyncQuotesDialog (GtkWindow *parent, GtkTreeModel *model, const gchar *fname, const gchar *tzone)
{
	// Operation status
	gboolean status = FALSE;

	// Get iterator position
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	{
		// Create time zone object
		TimeZone timezone;

		// Create client object for quote server
		Client client;

		// Create error object
		GError *error = NULL;

		// Load time zone and init quote client
		if (!timezone.Init (tzone, &error) || !client.Init (&error))
			ShowErrorMessage (GTK_WINDOW (parent), "Stock synchronization failed", error);
		else
		{
			// Create new sync list
			GtkListStore *list = gtk_list_store_new (SYNC_COLUMNS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT64, G_TYPE_INT64, G_TYPE_STRING);

			// Create good and bad lists
			GList *good = NULL;
			GList *bad = NULL;

			// Get stocks count
			gint records = 0;
			gint errors = 0;
			gint i = 0;
			gint size = GetTotalCount (GTK_TREE_MODEL (model));

			// Create progress dialog
			gboolean terminate = FALSE;
			ProgressDialog pwin = CreateProgressDialog (parent, "Syncing stock quotes...", &terminate);

			// Iterate through all elements
			do {
				// Get stock details
				gboolean state;
				gchar *ticker, *status = NULL;
				gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, STOCK_CHECK_ID, &state, STOCK_TICKER_ID, &ticker, -1);

				// Check if stock is marked
				if (state)
				{
					// Create error object
					GError *error = NULL;

					// Sync quotes
					SyncResult result = SyncQuotes (fname, ticker, &timezone, &client, &error);
					if (!result.status)
					{
						// Set status message
						status = g_strdup (error -> message);

						// Append new element to reference list
						bad = g_list_append (bad, gtk_tree_row_reference_new (GTK_TREE_MODEL (model), gtk_tree_model_get_path (GTK_TREE_MODEL (model), &iter)));

						// Release error object
						g_error_free (error);

						// Increment errors count
						errors++;
					}
					else
					{
						// Set status message
						status = g_strdup (STRING_OK);

						// Append new element to reference list
						good = g_list_append (good, gtk_tree_row_reference_new (GTK_TREE_MODEL (model), gtk_tree_model_get_path (GTK_TREE_MODEL (model), &iter)));
					}

					// Increment records count
					records++;

					// Add new element to list store object
					GtkTreeIter liter;
					gtk_list_store_append (GTK_LIST_STORE (list), &liter);
					gtk_list_store_set (GTK_LIST_STORE (list), &liter, SYNC_TICKER_ID, ticker, SYNC_QUOTES_ID, result.count, SYNC_START_ID, result.start, SYNC_END_ID, result.end, SYNC_STATUS_ID, status, -1);

					// Set current progress
					i++;
					gdouble fraction = static_cast <gdouble> (i) / size;
					if (GTK_IS_PROGRESS_BAR (pwin.progress))
						gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (pwin.progress), fraction);

					// Check if termination flag is set
					if (terminate)
					{
						// Clear check list
						gtk_list_store_clear (GTK_LIST_STORE (list));

						// Decrement reference count to check list
						g_object_unref (GTK_LIST_STORE (list));

						// Release lists of references
						g_list_free_full (good, reinterpret_cast <GDestroyNotify> (gtk_tree_row_reference_free));
						g_list_free_full (bad, reinterpret_cast <GDestroyNotify> (gtk_tree_row_reference_free));

						// Free temporary string buffers
						g_free (ticker);
						g_free (status);

						// Return terminate state
						return FALSE;
					}
				}

				// Free temporary string buffers
				g_free (ticker);
				g_free (status);

				// Process pending events
				while (gtk_events_pending ())
					gtk_main_iteration ();

				// Change iterator position to next element
			} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

			// Close progress window
			gtk_window_close (GTK_WINDOW (pwin.window));

			// Process pending events
			while (gtk_events_pending ())
				gtk_main_iteration ();

			// Show report dialog
			return CreateReportDialog (parent, "Sync report", "SyncReport.tsv", GTK_TREE_MODEL (model), GTK_TREE_MODEL (list), good, bad, CreateSyncList, SaveSyncReport, records, errors);
		}
	}

	// Return operation status
	return status;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
