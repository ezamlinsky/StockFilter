/*                                                                QuoteGraph.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                 QUOTE GRAPH                                  #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# include	<Common.h>
# include	<QuoteList.h>
# include	<Math.h>
# include	<math.h>		// TODO: Удалить как поменяю процессор

//****************************************************************************//
//      Internal constants                                                    //
//****************************************************************************//
# define	BUFFER_SIZE			512				// Static buffer size

//============================================================================//
//      Graph scale settings                                                  //
//============================================================================//
# define	SCALE_STEP			0.1				// Scale step for zoom in/out
# define	SCALE_MAX			2.0				// Max graph scale
# define	SCALE_MIN			0.3				// Min graph scale

//============================================================================//
//      Graph settings                                                        //
//============================================================================//
# define	FONT_FAMILY			"Sans"			// Font family
# define	BACKGROUND_COLOR	"#438FC3"		// Graph background color
# define	TEXT_COLOR			"#FFFFFF"		// Graph text color
# define	TEXT_SIZE			10				// Graph text size
# define	BAR_SIZE			9				// Default bar size
# define	BAR_PADDING			2				// Padding of bar box
# define	GRID_STEP			30				// Space between grid lines
# define	OUTLINE				4				// Outline size of drid lines
# define	SPACE				2				// Space between text and grid lines

//============================================================================//
//      Graph paddings                                                        //
//============================================================================//
# define	SEPARATOR			10				// Graph separator
# define	PADDING_TOP			20				// Graph top padding
# define	PADDING_BOTTOM		20				// Graph bottom padding
# define	PADDING_LEFT		10				// Graph left padding
# define	PADDING_RIGHT		70				// Graph right padding
# define	PADDING_LABEL		3				// Label paddings

//============================================================================//
//      Label colors                                                          //
//============================================================================//
# define	LABEL_BACKGROUND	"#FFFFFF"		// Label background color
# define	LABEL_COLOR			"#444444"		// Label text color

//============================================================================//
//      Line colors                                                           //
//============================================================================//
# define	BORDER_COLOR		"#7CB0DF"		// Border line color
# define	LINE_COLOR			"#6BA5D3"		// Line color
# define	BAR_COLOR			"#ACE6FF"		// Bar color

//============================================================================//
//      Line width                                                            //
//============================================================================//
# define	BORDER_WIDTH		2				// Border line width
# define	LINE_WIDTH			1				// Line width
# define	PRICE_WIDTH			3				// Price bar width
# define	VOLUME_WIDTH		5				// Volume bar width

//****************************************************************************//
//      Cursor structure                                                  //
//****************************************************************************//
struct Cursor
{
	gint		xpos;							// X cursor position
	gint		ypos;							// Y cursor position
};

//****************************************************************************//
//      Local objects                                                         //
//****************************************************************************//
static	GtkWidget	*window;
static	GtkWidget	*drawing;
static	GtkToolItem	*ZoomIn;
static	GtkToolItem	*ZoomOut;

//****************************************************************************//
//      Local variables                                                       //
//****************************************************************************//
static	gsize		shift;
static	gsize		psize;
static	gdouble		scale;
static	gint		xpos;
static	gint		ypos;
static	GdkRGBA		background;
static	GdkRGBA		text;
static	GdkRGBA		lbackground;
static	GdkRGBA		ltext;
static	GdkRGBA		border;
static	GdkRGBA		line;
static	GdkRGBA		bar;

//****************************************************************************//
//      Adjust cursor position to graph region                                //
//****************************************************************************//
static void AdjustCursor (Cursor *cursor, gint x, gint y, gint width, gint height)
{
	// Adjust cursor position
	cursor -> xpos -= x;
	cursor -> ypos -= y;

	// If cursor is outside area height
	if (static_cast <guint> (cursor -> ypos) >= static_cast <guint> (height))
	{
		// Set error value
		cursor -> ypos = -1;
	}

	// If cursor is outside area width
	if (static_cast <guint> (cursor -> xpos) >= static_cast <guint> (width))
	{
		// Set error value
		cursor -> xpos = -1;
		cursor -> ypos = -1;
	}
}

//****************************************************************************//
//      Round down value                                                      //
//****************************************************************************//
static gdouble RoundDown (gdouble value)
{
	// Get integer logarithm to base 10
	sint16_t log = static_cast <sint16_t> (Math::Log10 (value)) - 1;

	// Compute scale value
	gdouble scale = Math::Exp10i (log);

	// Return corrected value
	return floor (value / scale) * scale;			// TODO: Заменить как поменяю процессор
}

//****************************************************************************//
//      Round up value                                                        //
//****************************************************************************//
static gdouble RoundUp (gdouble value)
{
	// Get integer logarithm to base 10
	sint16_t log = static_cast <sint16_t> (Math::Log10 (value)) - 1;

	// Compute scale value
	gdouble scale = Math::Exp10i (log);

	// Return corrected value
	return ceil (value / scale) * scale;			// TODO: Заменить как поменяю процессор
}

//****************************************************************************//
//      Min quote price                                                       //
//****************************************************************************//
static gdouble MinPrice (const quote_t array[], gsize size)
{
	// Check all quotes
	gfloat price = +M_INF;
	while (size)
	{
		// Get min price
		price = Math::Min (array[0].low, price);
		array--;
		size--;
	}

	// Return min price
	return price;
}

//****************************************************************************//
//      Max quote price                                                       //
//****************************************************************************//
static gdouble MaxPrice (const quote_t array[], gsize size)
{
	// Check all quotes
	gfloat price = -M_INF;
	while (size)
	{
		// Get max price
		price = Math::Max (array[0].high, price);
		array--;
		size--;
	}

	// Return max price
	return price;
}

//****************************************************************************//
//      Max quote volume                                                      //
//****************************************************************************//
static gsize MaxVolume (const quote_t array[], gsize size)
{
	// Check all quotes
	gsize volume = 0;
	while (size)
	{
		// Get max volume
		volume = Math::Max (static_cast <uint64_t> (array[0].volume), static_cast <uint64_t> (volume));
		array--;
		size--;
	}

	// Return max volume
	return volume;
}

//****************************************************************************//
//      Adjust stock quotes                                                   //
//****************************************************************************//
static void AdjustQuotes (quote_t array[], gsize size)
{
	// Scale all prices
	while (size)
	{
		// Compute quote scale value
		gfloat scale = array[0].adjclose / array[0].close;

		// Adjust stock quote
		array[0].open *= scale;
		array[0].high *= scale;
		array[0].low *= scale;
		array[0].close *= scale;

		// Go to next quote
		array++;
		size--;
	}
}

//****************************************************************************//
//      Draw horizontal lines                                                 //
//****************************************************************************//
static void DrawHorizontalLines (cairo_t *cr, const gdouble array[], gsize size, gdouble shift, gdouble step, gdouble width, gdouble height, gdouble delta, gdouble base, gint cursor, gint precision)
{
	// Allocate space for static buffer
	gchar buffer [BUFFER_SIZE];

	// Create dash array
	const gdouble dashes[] = {3.0, 2.0};

	// Create text extent structure
	cairo_text_extents_t extent;

	// Set line style
	cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_source_rgb (cr, line.red, line.green, line.blue);
	cairo_set_line_width (cr, LINE_WIDTH);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
	cairo_set_dash (cr, dashes, sizeof (dashes) / sizeof (gdouble), 0);

	// Draw lines
	while (size)
	{
		// Draw line
		cairo_move_to (cr, width + OUTLINE, shift);
		cairo_rel_line_to (cr, -(width + OUTLINE), 0);
		cairo_stroke (cr);

		// Save cairo presets
		cairo_save (cr);

		// Set font style
		cairo_set_source_rgb (cr, text.red, text.green, text.blue);
		cairo_select_font_face (cr, FONT_FAMILY, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, TEXT_SIZE);

		// Draw label
		g_snprintf (buffer, BUFFER_SIZE, "%'.*f", precision, array[0]);
		cairo_text_extents (cr, buffer, &extent);
		cairo_move_to (cr, width + OUTLINE + SPACE, shift - 0.5 * extent.y_bearing);
		cairo_show_text (cr, buffer);

		// Restore cairo presets
		cairo_restore (cr);

		// Go to next line
		shift -= step;
		array++;
		size--;
	}

	// Check if measure line is visible
	if (cursor != -1)
	{
		// Set measure line style
		cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
		cairo_set_source_rgb (cr, border.red, border.green, border.blue);
		cairo_set_line_width (cr, LINE_WIDTH);
		cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
		cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
		cairo_set_dash (cr, NULL, 0, 0);

		// Draw measure line
		cairo_move_to (cr, width + OUTLINE, cursor);
		cairo_rel_line_to (cr, -(width + OUTLINE), 0);
		cairo_stroke (cr);

		// Set measure font style
		cairo_select_font_face (cr, FONT_FAMILY, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, TEXT_SIZE);

		// Prepare label text
		g_snprintf (buffer, BUFFER_SIZE, "%'.*f", precision, delta * (height - cursor) / height + base);
		cairo_text_extents (cr, buffer, &extent);

		// Compute label box position
		gdouble boxX = width + OUTLINE;
		gdouble boxY = cursor - 0.5 * extent.y_bearing + PADDING_LABEL;
		gdouble boxW = extent.width + 2.0 * PADDING_LABEL;
		gdouble boxH = -extent.y_bearing + 2.0 * PADDING_LABEL;

		// Draw label box
		cairo_set_source_rgb (cr, lbackground.red, lbackground.green, lbackground.blue);
		cairo_rectangle (cr, boxX, boxY, boxW, -boxH);
		cairo_fill (cr);

		// Draw label text
		cairo_set_source_rgb (cr, ltext.red, ltext.green, ltext.blue);
		cairo_move_to (cr, boxX - extent.x_bearing + PADDING_LABEL, boxY - PADDING_LABEL);
		cairo_show_text (cr, buffer);
	}
}

//****************************************************************************//
//      Draw vertical lines                                                   //
//****************************************************************************//
static void DrawVerticalLines (cairo_t *cr, const time_t array[], gsize size, gdouble shift, gdouble step, gdouble height)
{
	// Allocate space for static buffer
	gchar buffer [BUFFER_SIZE];

	// Create dash array
	const gdouble dashes[] = {3.0, 2.0};

	// Create text extent structure
	cairo_text_extents_t extent;

	// Set line style
	cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_source_rgb (cr, line.red, line.green, line.blue);
	cairo_set_line_width (cr, LINE_WIDTH);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
	cairo_set_dash (cr, dashes, sizeof (dashes) / sizeof (gdouble), 0);

	// Draw lines
	while (size)
	{
		// Check if time mark is set
		if (array[0] != static_cast <time_t> (TIME_ERROR))
		{
			// Draw long line
			cairo_move_to (cr, shift, height + OUTLINE);
			cairo_rel_line_to (cr, 0, -(height + OUTLINE));
			cairo_stroke (cr);

			// Save cairo presets
			cairo_save (cr);

			// Set font style
			cairo_set_source_rgb (cr, text.red, text.green, text.blue);
			cairo_select_font_face (cr, FONT_FAMILY, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
			cairo_set_font_size (cr, TEXT_SIZE);

			// Draw label
			date_struct date = Time::ExtractDate (array[0]);
			g_snprintf (buffer, BUFFER_SIZE, "%.4i-%.2d-%.2d", date.year, date.mon, date.day);
			cairo_text_extents (cr, buffer, &extent);
			cairo_move_to (cr, shift - extent.x_bearing - SPACE, height - extent.y_bearing + OUTLINE + SPACE);
			cairo_show_text (cr, buffer);

			// Restore cairo presets
			cairo_restore (cr);
		}
		else
		{
			// Draw short line
			cairo_move_to (cr, shift, height);
			cairo_rel_line_to (cr, 0, -height);
			cairo_stroke (cr);
		}

		// Go to next line
		shift += step;
		array++;
		size--;
	}
}

//****************************************************************************//
//      Draw price bars                                                       //
//****************************************************************************//
static void DrawPriceBars (cairo_t *cr, const quote_t array[], gsize size, gdouble shift, gdouble step, gdouble height, gdouble delta, gdouble base, gdouble scale, Cursor cursor)
{
	// Measure line draw status
	gboolean drawed = FALSE;

	// Set line style
	cairo_set_antialias (cr, CAIRO_ANTIALIAS_SUBPIXEL);
	cairo_set_line_width (cr, scale * PRICE_WIDTH);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
	cairo_set_dash (cr, NULL, 0, 0);

	// Draw bars
	while (size)
	{
		// Get quote prices
		gdouble open = height * (array[0].open - base) / delta;
		gdouble high = height * (array[0].high - base) / delta;
		gdouble low = height * (array[0].low - base) / delta;
		gdouble close = height * (array[0].close - base) / delta;

		// Set bar color
		cairo_set_source_rgb (cr, bar.red, bar.green, bar.blue);

		// Check if measure line is visible
		if (cursor.xpos != -1)
		{
			if (Math::Abs (shift - cursor.xpos) <= 0.5 * step && !drawed)
			{
				// Set measure line draw status
				drawed = TRUE;

				// Save cairo presets
				cairo_save (cr);

				// Set measure line style
				cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
				cairo_set_source_rgb (cr, border.red, border.green, border.blue);
				cairo_set_line_width (cr, LINE_WIDTH);
				cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
				cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
				cairo_set_dash (cr, NULL, 0, 0);

				// Draw measure line
				cairo_move_to (cr, shift, height);
				cairo_rel_line_to (cr, 0, -height);
				cairo_stroke (cr);

				// Restore cairo presets
				cairo_restore (cr);

				// Set active bar color
				cairo_set_source_rgb (cr, lbackground.red, lbackground.green, lbackground.blue);
			}
		}

		// Draw price bar
		cairo_move_to (cr, shift - scale * 0.5 * (BAR_SIZE - BAR_PADDING), height - open);
		cairo_line_to (cr, shift, height - open);
		cairo_line_to (cr, shift, height - high);
		cairo_line_to (cr, shift, height - low);
		cairo_line_to (cr, shift, height - close);
		cairo_line_to (cr, shift + scale * 0.5 * (BAR_SIZE - BAR_PADDING), height - close);
		cairo_stroke (cr);

		// Go to next bar
		shift += step;
		array--;
		size--;
	}
}

//****************************************************************************//
//      Draw volume bars                                                      //
//****************************************************************************//
static quote_t DrawVolumeBars (cairo_t *cr, const quote_t array[], gsize size, gdouble shift, gdouble step, gdouble height, gdouble delta, gdouble base, gdouble scale, Cursor cursor)
{
	// Allocate space for static buffer
	gchar buffer [BUFFER_SIZE];

	// Create quote details
	quote_t quote = {static_cast <time_t> (TIME_ERROR), 0, 0, 0, 0, 0, static_cast <gsize> (-1)};

	// Create text extent structure
	cairo_text_extents_t extent;

	// Measure line draw status
	gboolean drawed = FALSE;

	// Set line style
	cairo_set_antialias (cr, CAIRO_ANTIALIAS_SUBPIXEL);
	cairo_set_line_width (cr, scale * VOLUME_WIDTH);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
	cairo_set_dash (cr, NULL, 0, 0);

	// Draw bars
	while (size)
	{
		// Get qoute volume
		gdouble value = height * (array[0].volume - base) / delta;

		// Set bar color
		cairo_set_source_rgb (cr, bar.red, bar.green, bar.blue);

		// Check if measure line is visible
		if (cursor.xpos != -1)
		{
			if (Math::Abs (shift - cursor.xpos) <= 0.5 * step && !drawed)
			{
				// Set quote details
				quote = array[0];

				// Set measure line draw status
				drawed = TRUE;

				// Save cairo presets
				cairo_save (cr);

				// Set measure line style
				cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
				cairo_set_source_rgb (cr, border.red, border.green, border.blue);
				cairo_set_line_width (cr, LINE_WIDTH);
				cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
				cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
				cairo_set_dash (cr, NULL, 0, 0);

				// Draw measure line
				cairo_move_to (cr, shift, height + OUTLINE);
				cairo_rel_line_to (cr, 0, -(height + OUTLINE));
				cairo_stroke (cr);

				// Set measure font style
				cairo_select_font_face (cr, FONT_FAMILY, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
				cairo_set_font_size (cr, TEXT_SIZE);

				// Prepare label text
				date_struct date = Time::ExtractDate (array[0].date);
				g_snprintf (buffer, BUFFER_SIZE, "%.4i-%.2d-%.2d", date.year, date.mon, date.day);
				cairo_text_extents (cr, buffer, &extent);

				// Compute label box position
				gdouble boxX = ceil (shift - 0.5 * extent.width - PADDING_LABEL); // TODO: Заменить как поменяю процессор
				gdouble boxY = height - extent.y_bearing + 2.0 * PADDING_LABEL + OUTLINE;
				gdouble boxW = extent.width + 2.0 * PADDING_LABEL;
				gdouble boxH = -extent.y_bearing + 2.0 * PADDING_LABEL;

				// Draw label box
				cairo_set_source_rgb (cr, lbackground.red, lbackground.green, lbackground.blue);
				cairo_rectangle (cr, boxX, boxY, boxW, -boxH);
				cairo_fill (cr);

				// Draw label text
				cairo_set_source_rgb (cr, ltext.red, ltext.green, ltext.blue);
				cairo_move_to (cr, boxX - extent.x_bearing + PADDING_LABEL, boxY - PADDING_LABEL);
				cairo_show_text (cr, buffer);

				// Restore cairo presets
				cairo_restore (cr);

				// Set active bar color
				cairo_set_source_rgb (cr, lbackground.red, lbackground.green, lbackground.blue);
			}
		}

		// Draw volume bar
		cairo_move_to (cr, shift, height);
		cairo_rel_line_to (cr, 0, -value);
		cairo_stroke (cr);

		// Go to next bar
		shift += step;
		array--;
		size--;
	}

	// Return quote details
	return quote;
}

//****************************************************************************//
//      Draw quote details                                                    //
//****************************************************************************//
static void DrawQuoteDetails (cairo_t *cr, quote_t quote, gdouble width, gdouble height)
{
	// Check if time mark is set
	if (quote.date != static_cast <time_t> (TIME_ERROR))
	{
		// Allocate space for static buffer
		gchar buffer [BUFFER_SIZE];

		// Create text extent structure
		cairo_text_extents_t extent;

		// Set font style
		cairo_set_source_rgb (cr, text.red, text.green, text.blue);
		cairo_select_font_face (cr, FONT_FAMILY, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, TEXT_SIZE);

		// Prepare quote detail text
		date_struct date = Time::ExtractDate (quote.date);
		g_snprintf (buffer, BUFFER_SIZE, "Date: %.4i-%.2d-%.2d    Open: %.2f    High: %.2f    Low: %.2f    Close: %.2f    Volume: %'ld", date.year, date.mon, date.day, quote.open, quote.high, quote.low, quote.close, quote.volume);
		cairo_text_extents (cr, buffer, &extent);

		// Draw quote detail text
		cairo_move_to (cr, PADDING_LEFT - extent.x_bearing, PADDING_TOP - (OUTLINE + SPACE));
		cairo_show_text (cr, buffer);
	}
}

//****************************************************************************//
//      Draw scale factor                                                     //
//****************************************************************************//
static void DrawScaleFactor (cairo_t *cr, gdouble scale, gdouble width, gdouble height)
{
	// Allocate space for static buffer
	gchar buffer [BUFFER_SIZE];

	// Create text extent structure
	cairo_text_extents_t extent;

	// Set font style
	cairo_set_source_rgb (cr, text.red, text.green, text.blue);
	cairo_select_font_face (cr, FONT_FAMILY, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, TEXT_SIZE);

	// Prepare scale factor text
	g_snprintf (buffer, BUFFER_SIZE, "Scale: %.2f", scale);
	cairo_text_extents (cr, buffer, &extent);

	// Draw scale factor
	cairo_move_to (cr, width - PADDING_RIGHT - extent.width, PADDING_TOP - (OUTLINE + SPACE));
	cairo_show_text (cr, buffer);
}

//****************************************************************************//
//      Draw price graph                                                      //
//****************************************************************************//
static void DrawPriceGraph (cairo_t *cr, const quote_t array[], gsize size, gdouble bsize, gdouble width, gdouble height, gdouble scale, Cursor cursor)
{
	// Get min and max prices
	gdouble min = RoundDown (MinPrice (array, size));
	gdouble max = RoundUp (MaxPrice (array, size));
	gdouble delta = max - min;

	// Compute price step for price grid
	gdouble step = RoundUp (delta * GRID_STEP / height);

	// Create horizontal lines data
	gsize hlines = delta / step;
	gdouble harray [hlines];
	gdouble hstep = height * step / delta;
	gdouble hshift = height - hstep;

	// Fill horizontal lines array
	gdouble value = min + step;
	for (gsize i = 0; i < hlines; i++)
	{
		harray[i] = value;
		value += step;
	}

	// Draw horizontal lines
	DrawHorizontalLines (cr, harray, hlines, hshift, hstep, width, height, delta, min, cursor.ypos, 2);

	// Create vertical lines data
	gsize bcount = ceil (GRID_STEP / bsize);	// TODO: Заменить как поменяю процессор
	gsize vlines = ceil (static_cast <gdouble> (size) / bcount);	// TODO: Заменить как поменяю процессор
	time_t varray [vlines];
	gdouble vstep = bsize * bcount;
	gdouble vshift = 0.5 * bsize;

	// Fill vertical lines array
	for (size_t i = 0; i < vlines; i++)
		varray[i] = TIME_ERROR;

	// Draw vertical lines
	DrawVerticalLines (cr, varray, vlines, vshift, vstep, height);

	// Draw price bars
	DrawPriceBars (cr, array, size, 0.5 * bsize, bsize, height, delta, min, scale, cursor);

	// Show graph frame
	cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_source_rgb (cr, border.red, border.green, border.blue);
	cairo_set_line_width (cr, BORDER_WIDTH);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
	cairo_set_dash (cr, NULL, 0, 0);
	cairo_rectangle (cr, 0.0, 0.0, width, height);
	cairo_stroke (cr);
}

//****************************************************************************//
//      Draw volume graph                                                     //
//****************************************************************************//
static quote_t DrawVolumeGraph (cairo_t *cr, const quote_t array[], gsize size, gdouble bsize, gdouble width, gdouble height, gdouble scale, Cursor cursor)
{
	// Get max volume
	gsize min = 0;
	gsize max = RoundUp (MaxVolume (array, size));
	gsize delta = max - min;

	// Compute volume step for volume grid
	gsize step = RoundUp (delta * GRID_STEP / height);

	// Create horizontal lines data
	gsize hlines = delta / step;
	gdouble harray [hlines];
	gdouble hstep = height * step / delta;
	gdouble hshift = height - hstep;

	// Fill horizontal lines array
	gsize value = min + step;
	for (size_t i = 0; i < hlines; i++)
	{
		harray[i] = value;
		value += step;
	}

	// Draw horizontal lines
	DrawHorizontalLines (cr, harray, hlines, hshift, hstep, width, height, delta, min, cursor.ypos, 0);

	// Create vertical lines data
	gsize bcount = ceil (GRID_STEP / bsize);	// TODO: Заменить как поменяю процессор
	gsize vlines = ceil (static_cast <gdouble> (size) / bcount);	// TODO: Заменить как поменяю процессор
	time_t varray [vlines];
	gdouble vstep = bsize * bcount;
	gdouble vshift = 0.5 * bsize;

	// Fill vertical lines array
	gint index = 0;
	for (gsize i = 0; i < vlines; i++)
	{
		if (i % 3)
			varray[i] = TIME_ERROR;
		else
			varray[i] = array[index].date;
		index -= bcount;
	}

	// Draw vertical lines
	DrawVerticalLines (cr, varray, vlines, vshift, vstep, height);

	// Draw volume bars
	quote_t quote = DrawVolumeBars (cr, array, size, 0.5 * bsize, bsize, height, delta, min, scale, cursor);

	// Show graph frame
	cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_source_rgb (cr, border.red, border.green, border.blue);
	cairo_set_line_width (cr, BORDER_WIDTH);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
	cairo_set_dash (cr, NULL, 0, 0);
	cairo_rectangle (cr, 0.0, 0.0, width, height);
	cairo_stroke (cr);

	// Return quote details
	return quote;
}

//****************************************************************************//
//      Signal handler for "draw" event of drawing area                       //
//****************************************************************************//
static gboolean DrawGraph (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	// Convert data pointer
	Quotes *quotes = reinterpret_cast <Quotes*> (data);

	// Create cursor structure
	Cursor cursor;

	// Get quote list
	QuoteList list = quotes -> GetQuoteList ();

	// Adjust stock quotes
	AdjustQuotes (list.array, list.size);

	// Get widget parameters
	gint width = gtk_widget_get_allocated_width (GTK_WIDGET (widget));
	gint height = gtk_widget_get_allocated_height (GTK_WIDGET (widget));

	// Compute graphs width and height
	gdouble gwidth = width - PADDING_LEFT - PADDING_RIGHT;
	gdouble vheight = (height - PADDING_TOP - PADDING_BOTTOM - SEPARATOR) / 6;
	gdouble pheight = height - PADDING_TOP - PADDING_BOTTOM - SEPARATOR - vheight;

	// Compute bar size
	gdouble	bsize = scale * BAR_SIZE;

	// Compute quotes count to display
	psize = gwidth / bsize;
	gsize count = psize;
	if (count > list.size)
		count = list.size;

	// Correct graph shift
	if (shift > list.size - count)
		shift = list.size - count;

	// Set background
	cairo_set_source_rgb (cr, background.red, background.green, background.blue);
	cairo_paint (cr);

	// Draw price graph
	cairo_save (cr);
	cairo_translate(cr, PADDING_LEFT, PADDING_TOP);
	cursor = {xpos, ypos};
	AdjustCursor (&cursor, PADDING_LEFT, PADDING_TOP, gwidth, pheight);
	DrawPriceGraph (cr, list.array + shift + count - 1, count, bsize, gwidth, pheight, scale, cursor);
	cairo_restore (cr);

	// Draw volume graph
	cairo_save (cr);
	cairo_translate(cr, PADDING_LEFT, pheight + PADDING_TOP + SEPARATOR);
	cursor = {xpos, ypos};
	AdjustCursor (&cursor, PADDING_LEFT, pheight + PADDING_TOP + SEPARATOR, gwidth, vheight);
	quote_t quote = DrawVolumeGraph (cr, list.array + shift + count - 1, count, bsize, gwidth, vheight, scale, cursor);
	cairo_restore (cr);

	// Draw quote details
	DrawQuoteDetails (cr, quote, width, height);

	// Draw scale factor
	DrawScaleFactor (cr, scale, width, height);

	// Return process event status
	return FALSE;
}

//****************************************************************************//
//      Signal handler for "motion-notify-event" event of dialog window       //
//****************************************************************************//
static gboolean MouseOver (GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	// Get mouse pointer coordinates
	gtk_widget_get_pointer (GTK_WIDGET (drawing), &xpos, &ypos);

	// Get widget parameters
	gint width = gtk_widget_get_allocated_width (GTK_WIDGET (drawing));
	gint height = gtk_widget_get_allocated_height (GTK_WIDGET (drawing));

	// Correct cursor position
	if (static_cast <guint> (xpos) < static_cast <guint> (width) && static_cast <guint> (ypos) < static_cast <guint> (height))
	{
		// Redraw drawing area
		gtk_widget_queue_draw (GTK_WIDGET (drawing));
	}

	// Return process event status
	return FALSE;
}

//****************************************************************************//
//      Signal handler for "FirstBar" tool button                             //
//****************************************************************************//
static void GoFirstBar (GtkToolButton *toolbutton, gpointer data)
{
	// Change graph shift value
	shift = static_cast <gsize> (-1);

	// Clear mouse pointer coordinates
	xpos = -1;
	ypos = -1;

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//****************************************************************************//
//      Signal handler for "PreviousPage" tool button                         //
//****************************************************************************//
static void GoPreviousPage (GtkToolButton *toolbutton, gpointer data)
{
	// Change graph shift value
	if (shift <= static_cast <gsize> (-1) - psize)
		shift += psize;

	// Clear mouse pointer coordinates
	xpos = -1;
	ypos = -1;

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//****************************************************************************//
//      Signal handler for "PreviousBar" tool button                          //
//****************************************************************************//
static void GoPreviousBar (GtkToolButton *toolbutton, gpointer data)
{
	// Change graph shift value
	if (shift <= static_cast <gsize> (-1) - 1)
		shift += 1;

	// Clear mouse pointer coordinates
	xpos = -1;
	ypos = -1;

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//****************************************************************************//
//      Signal handler for "NextBar" tool button                              //
//****************************************************************************//
static void GoNextBar (GtkToolButton *toolbutton, gpointer data)
{
	// Change graph shift value
	if (shift >= static_cast <gsize> (0) + 1)
		shift -= 1;

	// Clear mouse pointer coordinates
	xpos = -1;
	ypos = -1;

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//****************************************************************************//
//      Signal handler for "NextPage" tool button                             //
//****************************************************************************//
static void GoNextPage (GtkToolButton *toolbutton, gpointer data)
{
	// Change graph shift value
	if (shift >= static_cast <gsize> (0) + psize)
		shift -= psize;

	// Clear mouse pointer coordinates
	xpos = -1;
	ypos = -1;

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//****************************************************************************//
//      Signal handler for "LastBar" tool button                              //
//****************************************************************************//
static void GoLastBar (GtkToolButton *toolbutton, gpointer data)
{
	// Change graph shift value
	shift = static_cast <gsize> (0);

	// Clear mouse pointer coordinates
	xpos = -1;
	ypos = -1;

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//****************************************************************************//
//      Signal handler for "ZoomIn" tool button                               //
//****************************************************************************//
static void ZoomInGraph (GtkToolButton *toolbutton, gpointer data)
{
	// Change graph scale value
	scale += SCALE_STEP;

	// If reached max scale
	if (scale >= SCALE_MAX)
	{
		// Then disable zoom in button
		gtk_widget_set_sensitive (GTK_WIDGET (ZoomIn), FALSE);
	}

	// Enable zoom out button
	gtk_widget_set_sensitive (GTK_WIDGET (ZoomOut), TRUE);

	// Clear mouse pointer coordinates
	xpos = -1;
	ypos = -1;

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//****************************************************************************//
//      Signal handler for "ZoomOut" tool button                              //
//****************************************************************************//
static void ZoomOutGraph (GtkToolButton *toolbutton, gpointer data)
{
	// Change graph scale value
	scale -= SCALE_STEP;

	// If reached max scale
	if (scale <= SCALE_MIN)
	{
		// Then disable zoom out button
		gtk_widget_set_sensitive (GTK_WIDGET (ZoomOut), FALSE);
	}

	// Enable zoom in button
	gtk_widget_set_sensitive (GTK_WIDGET (ZoomIn), TRUE);

	// Clear mouse pointer coordinates
	xpos = -1;
	ypos = -1;

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//****************************************************************************//
//      Signal handler for "Zoom100" tool button                              //
//****************************************************************************//
static void Zoom100Graph (GtkToolButton *toolbutton, gpointer data)
{
	// Change graph scale value
	scale = 1.0;

	// Enable zoom in and zoom out buttons
	gtk_widget_set_sensitive (GTK_WIDGET (ZoomIn), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (ZoomOut), TRUE);

	// Clear mouse pointer coordinates
	xpos = -1;
	ypos = -1;

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//****************************************************************************//
//      Tool bar                                                              //
//****************************************************************************//
static GtkWidget* CreateToolBar (void)
{
	// Create tool bar
	GtkWidget *toolbar = gtk_toolbar_new ();

	// Create buttons
	GtkToolItem *FirstBar = gtk_tool_button_new (NULL, "First bar");
	GtkToolItem *PreviousPage = gtk_tool_button_new (NULL, "Previous page");
	GtkToolItem *PreviousBar = gtk_tool_button_new (NULL, "Previous bar");
	GtkToolItem *NextBar = gtk_tool_button_new (NULL, "Next bar");
	GtkToolItem *NextPage = gtk_tool_button_new (NULL, "Next page");
	GtkToolItem *LastBar = gtk_tool_button_new (NULL, "Last bar");
	GtkToolItem *Separator = gtk_separator_tool_item_new();
	ZoomIn = gtk_tool_button_new (NULL, "Zoom in");
	ZoomOut = gtk_tool_button_new (NULL, "Zoom out");
	GtkToolItem *Zoom100 = gtk_tool_button_new (NULL, "Zoom 100%");

	// Add buttons to tool bar
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (FirstBar), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (PreviousPage), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (PreviousBar), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (NextBar), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (NextPage), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (LastBar), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Separator), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (ZoomIn), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (ZoomOut), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Zoom100), -1);

	// Add tooltip text to buttons
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (FirstBar), "Go to first bar");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (PreviousPage), "Go to previous page");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (PreviousBar), "Go to previous bar");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (NextBar), "Go to next bar");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (NextPage), "Go to next page");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (LastBar), "Go to last bar");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (ZoomIn), "Zoom in");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (ZoomOut), "Zoom out");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Zoom100), "Zoom in 100%");
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (FirstBar), TRUE);
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (PreviousPage), FALSE);
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (PreviousBar), FALSE);
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (NextBar), FALSE);
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (NextPage), FALSE);
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (LastBar), TRUE);
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (ZoomIn), FALSE);
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (ZoomOut), FALSE);
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (Zoom100), FALSE);

	// Set button properties
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (FirstBar), "media-skip-backward");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (PreviousPage), "go-first");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (PreviousBar), "go-previous");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (NextBar), "go-next");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (NextPage), "go-last");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (LastBar), "media-skip-forward");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (ZoomIn), "gtk-zoom-in");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (ZoomOut), "gtk-zoom-out");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Zoom100), "gtk-zoom-100");
	gtk_tool_item_set_expand (GTK_TOOL_ITEM (Separator), TRUE);

	// Set tool bar properties
	gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), TRUE);
	gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH_HORIZ);
	gtk_toolbar_set_icon_size (GTK_TOOLBAR (toolbar), GTK_ICON_SIZE_LARGE_TOOLBAR);

	// Assign signal handlers
	g_signal_connect (G_OBJECT (FirstBar), "clicked", G_CALLBACK (GoFirstBar), NULL);
	g_signal_connect (G_OBJECT (PreviousPage), "clicked", G_CALLBACK (GoPreviousPage), NULL);
	g_signal_connect (G_OBJECT (PreviousBar), "clicked", G_CALLBACK (GoPreviousBar), NULL);
	g_signal_connect (G_OBJECT (NextBar), "clicked", G_CALLBACK (GoNextBar), NULL);
	g_signal_connect (G_OBJECT (NextPage), "clicked", G_CALLBACK (GoNextPage), NULL);
	g_signal_connect (G_OBJECT (LastBar), "clicked", G_CALLBACK (GoLastBar), NULL);
	g_signal_connect (G_OBJECT (ZoomIn), "clicked", G_CALLBACK (ZoomInGraph), NULL);
	g_signal_connect (G_OBJECT (ZoomOut), "clicked", G_CALLBACK (ZoomOutGraph), NULL);
	g_signal_connect (G_OBJECT (Zoom100), "clicked", G_CALLBACK (Zoom100Graph), NULL);

	// Return tool bar
	return toolbar;
}

//****************************************************************************//
//      Drawing area                                                          //
//****************************************************************************//
static GtkWidget* CreateDrawingArea (Quotes *quotes)
{
	// Create drawing area
	drawing = gtk_drawing_area_new ();

	// Set drawing area properties
	gtk_widget_set_size_request (GTK_WIDGET (drawing), 800, 600);

	// Set default shift
	shift = 0;

	// Set default scale
	scale = 1.0;

	// Set drawing colors
	gdk_rgba_parse  (&background, BACKGROUND_COLOR);
	gdk_rgba_parse  (&text, TEXT_COLOR);
	gdk_rgba_parse  (&lbackground, LABEL_BACKGROUND);
	gdk_rgba_parse  (&ltext, LABEL_COLOR);
	gdk_rgba_parse  (&border, BORDER_COLOR);
	gdk_rgba_parse  (&line, LINE_COLOR);
	gdk_rgba_parse  (&bar, BAR_COLOR);

	// Assign signal handlers
	g_signal_connect (G_OBJECT (drawing), "draw", G_CALLBACK (DrawGraph), quotes);

	// Return drawing area
	return drawing;
}

//****************************************************************************//
//      View quotes dialog                                                    //
//****************************************************************************//
gboolean ViewQuotesDialog (GtkWindow *parent, const gchar *path, const gchar *ticker, const gchar *name, const gchar *country, const gchar *sector, const gchar *industry, const gchar *url)
{
	// Operation status
	gboolean status = FALSE;

	// Create quotes object
	Quotes quotes;

	// Create error object
	GError *error = NULL;

	// Try to open quotes
	if (!quotes.OpenList (path, &error))
		ShowFileErrorMessage (GTK_WINDOW (parent), "Can not open stock quotes", error);
	else
	{
		// Extract file name from path
		gchar *fname = g_filename_display_basename (path);

		// Create string buffer
		GString *string = g_string_new (NULL);

		// Create title bar string
		g_string_printf (string, "View stock quotes - %s", fname);

		// Create dialog window
		window = gtk_dialog_new_with_buttons (string -> str, GTK_WINDOW (parent), GTK_DIALOG_MODAL, "_Close", GTK_RESPONSE_CLOSE, NULL);

		// Get content area of dialog
		GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (window));

		// Get action area of dialog
		GtkWidget *action = gtk_dialog_get_action_area (GTK_DIALOG (window));

		// Add items to box
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateToolBar ()), FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateDrawingArea (&quotes)), TRUE, TRUE, 0);
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateStockSummary (ticker, name, country, sector, industry, url, gtk_container_get_border_width (GTK_CONTAINER (box)), gtk_container_get_border_width (GTK_CONTAINER (action)))), FALSE, FALSE, 0);

		// Assign signal handlers
		g_signal_connect (G_OBJECT (window), "motion-notify-event", G_CALLBACK (MouseOver), NULL);

		// Set default dialog button
		gtk_dialog_set_default_response (GTK_DIALOG (window), GTK_RESPONSE_ACCEPT);

		// Show all box elements
		gtk_widget_show_all (GTK_WIDGET (box));

		// Run dialog window
		gtk_dialog_run (GTK_DIALOG (window));

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
