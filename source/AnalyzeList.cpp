/*                                                               AnalyzeList.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                 ANALYZE LIST                                 #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# include	<Common.h>
# include	<Quotes.h>
# include	<StockList.h>
# include	<AnalyzeList.h>

//****************************************************************************//
//      Analyze result structure                                              //
//****************************************************************************//
struct AnalyzeResult
{
	gboolean	status;			// Analyze status
	time_t		date;			// Last quote date
	time_t		sync;			// Sync time
	gint		count;			// Quotes count
	gsize		liquidity;		// Stock liquidity
	gfloat		volatility;		// Stock volatility
	gfloat		price;			// Last quote price
};

//****************************************************************************//
//      Analyze stock quotes for trading                                      //
//****************************************************************************//
static AnalyzeResult AnalyzeQuotes (const gchar *fname, const gchar* ticker, gint min_count, gsize min_liquidity, gfloat min_volatility, gfloat min_price, GError **error)
{
	// Init result structure
	AnalyzeResult result = {
		static_cast <gboolean> (FALSE),
		static_cast <time_t> (TIME_ERROR),
		static_cast <time_t> (TIME_ERROR),
		static_cast <gint> (-1),
		static_cast <gsize> (-1),
		static_cast <gfloat> (-1),
		static_cast <gfloat> (-1)
	};

	// Get quotes file name
	gchar* path = GetQuotesFile (fname, ticker);

	// Create quotes object
	Quotes quotes;

	// Try to open quotes
	if (quotes.OpenList (path, error))
	{
		// Get stock details
		result.date = quotes.GetLastDate ();
		result.sync = quotes.GetSyncTime ();
		result.count = quotes.GetCount ();
		result.liquidity = quotes.GetLiquidity (min_count);
		result.volatility = quotes.GetVolatility (min_count);
		result.price = quotes.GetLastPrice ();

		// Check quotes count
		if (result.count >= min_count)
		{
			// Check stock liquidity
			if (result.liquidity >= min_liquidity)
			{
				// Check stock volatility
				if (result.volatility >= min_volatility)
				{
					// Check stock price
					if (result.price >= min_price)
					{
						// Set result status
						result.status = TRUE;
					}
					else
					{
						// Set error message
						g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Filtered out by stock price");
					}
				}
				else
				{
					// Set error message
					g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Filtered out by stock volatility");
				}
			}
			else
			{
				// Set error message
				g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Filtered out by stock liquidity");
			}
		}
		else
		{
			// Set error message
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Filtered out by quotes count");
		}
	}

	// Free temporary string buffer
	g_free (path);

	// Normal exit
	return result;
}

//****************************************************************************//
//      Save analyze report function                                          //
//****************************************************************************//
static gboolean SaveAnalyzeReport (const gchar *fname, GtkTreeModel *model, GError **error)
{
	// Create string buffer
	GString *string = g_string_new (NULL);

	// Check if report is set
	if (model)
	{
		// Append report header
		g_string_append_printf (string, ANALYZE_TICKER_LABEL "\t" ANALYZE_DATE_LABEL "\t" ANALYZE_SYNC_LABEL "\t" ANALYZE_QUOTES_LABEL "\t" ANALYZE_LIQUIDITY_LABEL "\t" ANALYZE_VOLATILITY_LABEL "\t" ANALYZE_PRICE_LABEL "\t" ANALYZE_STATUS_LABEL "\n");

		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
		{
			// Iterate through all elements
			do {
				// Get record details
				gchar *ticker, *status;
				time_t date, sync;
				gint count;
				gsize liquidity;
				gfloat volatility, price;
				gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, ANALYZE_TICKER_ID, &ticker, ANALYZE_DATE_ID, &date, ANALYZE_SYNC_ID, &sync, ANALYZE_QUOTES_ID, &count, ANALYZE_LIQUIDITY_ID, &liquidity, ANALYZE_VOLATILITY_ID, &volatility, ANALYZE_PRICE_ID, &price, ANALYZE_STATUS_ID, &status, -1);

				// Append quote information into string buffer
				date_struct ldate = Time::ExtractDate (date);
				date_struct sdate = Time::ExtractDate (sync);
				g_string_append_printf (string, "%s\t%.4i-%.2d-%.2d\t%.4i-%.2d-%.2d\t%i\t%lli\t%.6f\t%.2f\t%s\n", ticker, ldate.year, ldate.mon, ldate.day, sdate.year, sdate.mon, sdate.day, count, liquidity, volatility, price, status);

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
//      Analyze list                                                          //
//****************************************************************************//
static GtkWidget* CreateAnalyzeList (GtkTreeModel *model)
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
	GtkCellRenderer *DateCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *SyncCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *QuotesCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *LiquidityCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *VolatilityCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *PriceCell = gtk_cell_renderer_text_new ();
	GtkCellRenderer *StatusCell = gtk_cell_renderer_text_new ();

	// Create columns for tree view
	GtkTreeViewColumn *TickerColumn = gtk_tree_view_column_new_with_attributes (ANALYZE_TICKER_LABEL, GTK_CELL_RENDERER (TickerCell), "text", ANALYZE_TICKER_ID, NULL);
	GtkTreeViewColumn *DateColumn = gtk_tree_view_column_new_with_attributes (ANALYZE_DATE_LABEL, GTK_CELL_RENDERER (DateCell), "text", ANALYZE_DATE_ID, NULL);
	GtkTreeViewColumn *SyncColumn = gtk_tree_view_column_new_with_attributes (ANALYZE_SYNC_LABEL, GTK_CELL_RENDERER (SyncCell), "text", ANALYZE_SYNC_ID, NULL);
	GtkTreeViewColumn *QuotesColumn = gtk_tree_view_column_new_with_attributes (ANALYZE_QUOTES_LABEL, GTK_CELL_RENDERER (QuotesCell), "text", ANALYZE_QUOTES_ID, NULL);
	GtkTreeViewColumn *LiquidityColumn = gtk_tree_view_column_new_with_attributes (ANALYZE_LIQUIDITY_LABEL, GTK_CELL_RENDERER (LiquidityCell), "text", ANALYZE_LIQUIDITY_ID, NULL);
	GtkTreeViewColumn *VolatilityColumn = gtk_tree_view_column_new_with_attributes (ANALYZE_VOLATILITY_LABEL, GTK_CELL_RENDERER (VolatilityCell), "text", ANALYZE_VOLATILITY_ID, NULL);
	GtkTreeViewColumn *PriceColumn = gtk_tree_view_column_new_with_attributes (ANALYZE_PRICE_LABEL, GTK_CELL_RENDERER (PriceCell), "text", ANALYZE_PRICE_ID, NULL);
	GtkTreeViewColumn *StatusColumn = gtk_tree_view_column_new_with_attributes (ANALYZE_STATUS_LABEL, GTK_CELL_RENDERER (StatusCell), "text", ANALYZE_STATUS_ID, NULL);

	// Add columns to tree view
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (TickerColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (DateColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (SyncColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (QuotesColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (LiquidityColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (VolatilityColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (PriceColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (StatusColumn));

	// Add tree view to scrolled window
	gtk_container_add (GTK_CONTAINER (scrolled), GTK_WIDGET (treeview));

	// Set cell properties
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (TickerCell), 0.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (DateCell), 0.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (SyncCell), 0.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (QuotesCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (LiquidityCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (VolatilityCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (PriceCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (StatusCell), 0.0, 0.0);

	// Set column properties
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (TickerColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (DateColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (SyncColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (QuotesColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (LiquidityColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (VolatilityColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (PriceColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (StatusColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (TickerColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (DateColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (SyncColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (QuotesColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (LiquidityColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (VolatilityColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (PriceColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (StatusColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (TickerColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (DateColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (SyncColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (QuotesColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (LiquidityColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (VolatilityColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (PriceColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (StatusColumn), TRUE);

	// Set tree view sort properties
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (TickerColumn), ANALYZE_TICKER_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (DateColumn), ANALYZE_DATE_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (SyncColumn), ANALYZE_SYNC_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (QuotesColumn), ANALYZE_QUOTES_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (LiquidityColumn), ANALYZE_LIQUIDITY_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (VolatilityColumn), ANALYZE_VOLATILITY_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (PriceColumn), ANALYZE_PRICE_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (StatusColumn), ANALYZE_STATUS_ID);

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
	gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (scrolled), 1300);
	gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (scrolled), 600);

	// Assign cell data functions
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (DateColumn), GTK_CELL_RENDERER (DateCell), DateRenderFunc, GUINT_TO_POINTER (ANALYZE_DATE_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (SyncColumn), GTK_CELL_RENDERER (SyncCell), DateRenderFunc, GUINT_TO_POINTER (ANALYZE_SYNC_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (QuotesColumn), GTK_CELL_RENDERER (QuotesCell), CountRenderFunc, GUINT_TO_POINTER (ANALYZE_QUOTES_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (LiquidityColumn), GTK_CELL_RENDERER (LiquidityCell), CountRenderFunc, GUINT_TO_POINTER (ANALYZE_LIQUIDITY_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (VolatilityColumn), GTK_CELL_RENDERER (VolatilityCell), PercentRenderFunc, GUINT_TO_POINTER (ANALYZE_VOLATILITY_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (PriceColumn), GTK_CELL_RENDERER (PriceCell), PriceRenderFunc, GUINT_TO_POINTER (ANALYZE_PRICE_ID), NULL);

	// Set search column for immediate search
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), ANALYZE_TICKER_ID);

	// Set tree view model
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (model));

	// Return scrolled window object
	return scrolled;
}

//****************************************************************************//
//      Analyze quotes dialog                                                 //
//****************************************************************************//
gboolean AnalyzeQuotesDialog (GtkWindow *parent, GtkTreeModel *model, const gchar *fname, gint count, gfloat liquidity, gfloat volatility, gfloat price)
{
	// Operation status
	gboolean status = FALSE;

	// Get iterator position
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	{
		// Create new analyze list
		GtkListStore *list = gtk_list_store_new (ANALYZE_COLUMNS, G_TYPE_STRING, G_TYPE_INT64, G_TYPE_INT64, G_TYPE_INT, G_TYPE_UINT64, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_STRING);

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
		ProgressDialog pwin = CreateProgressDialog (parent, "Analyzing stock quotes...", &terminate);

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

				// Analyze quotes
				AnalyzeResult result = AnalyzeQuotes (fname, ticker, count, liquidity, volatility, price, &error);
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
				gtk_list_store_set (GTK_LIST_STORE (list), &liter, ANALYZE_TICKER_ID, ticker, ANALYZE_DATE_ID, result.date, ANALYZE_SYNC_ID, result.sync, ANALYZE_QUOTES_ID, result.count, ANALYZE_LIQUIDITY_ID, result.liquidity, ANALYZE_VOLATILITY_ID, result.volatility, ANALYZE_PRICE_ID, result.price, ANALYZE_STATUS_ID, status, -1);

				// Set current progress
				i++;
				gdouble fraction = static_cast <gdouble> (i) / size;
				if (GTK_IS_PROGRESS_BAR (pwin.progress))
					gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (pwin.progress), fraction);

				// Check if termination flag is set
				if (terminate)
				{
					// Clear analyze list
					gtk_list_store_clear (GTK_LIST_STORE (list));

					// Decrement reference count to analyze list
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
		return CreateReportDialog (parent, "Analyze report", "AnalyzeReport.tsv", GTK_TREE_MODEL (model), GTK_TREE_MODEL (list), good, bad, CreateAnalyzeList, SaveAnalyzeReport, records, errors);
	}

	// Return operation status
	return status;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
