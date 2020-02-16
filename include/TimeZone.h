/*                                                                    TimeZone.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                               TIME ZONE CLASS                                #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>
# include	<Time.h>

//****************************************************************************//
//      Time zone class                                                       //
//****************************************************************************//
class TimeZone
{
private:
	Time	time;			// Time zone object

public:

	// Loading time zone
	gboolean Init (const gchar *tzone, GError **error);

	// Get current time in time zone
	time_t GetCurrentTime (void);
};
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
