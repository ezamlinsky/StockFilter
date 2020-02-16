/*                                                                      Common.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                               COMMON FUNCTIONS                               #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>

//****************************************************************************//
//      Global constants                                                      //
//****************************************************************************//
# define	PROGRAM_NAME	"Stock Filter"
# define	LOGO_FILE		"/usr/share/icons/hicolor/scalable/apps/stock-filter.svg"
# define	STRING_OK		"OK"
# define	STRING_UNKNOWN	"Unknown"

//****************************************************************************//
//      Status list structure                                                 //
//****************************************************************************//
struct StatusList
{
	GtkTreeModel	*model;			// Stock list
	GList			*list;			// List of chosen stocks
};

//****************************************************************************//
//      Progress dialog structure                                             //
//****************************************************************************//
struct ProgressDialog
{
	GtkWindow		*window;		// Progress dialog window
	GtkProgressBar	*progress;		// Progress bar
};

//****************************************************************************//
//      Function prototypes                                                   //
//****************************************************************************//
void DateRenderFunc (GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
void PriceRenderFunc (GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
void PercentRenderFunc (GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
void CountRenderFunc (GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
void ShowStatusMessage (const gchar *message, guint context);
void ShowErrorMessage (GtkWindow *window, const gchar *message, GError *error);
void ShowFileErrorMessage (GtkWindow *window, const gchar *message, GError *error);
gint GetTotalCount (GtkTreeModel *model);
gchar* GetQuotesFile (const gchar *path, const gchar *ticker);
GtkWidget* CreateStockSummary (const gchar *ticker, const gchar *name, const gchar *country, const gchar *sector, const gchar *industry, const gchar *url, guint box_border, guint action_border);
ProgressDialog CreateProgressDialog (GtkWindow *parent, const gchar *message, gboolean *flag);
gboolean CreateReportDialog (GtkWindow *parent, const gchar *title, const gchar *rname, GtkTreeModel *stocks, GtkTreeModel *report, GList *good, GList *bad, GtkWidget* (*CreateList) (GtkTreeModel *model), gboolean (*SaveReport) (const gchar *fname, GtkTreeModel *model, GError **error), gint total, gint errors);
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
