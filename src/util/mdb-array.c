/* MDB Tools - A library for reading MS Access database file
 * Copyright  (C) 2000 Brian Bruns
 *
 *
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or  (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* this is inherited from mdb-export.c, modified to make arrays similar to
   those generated by parsecsv.c */

#include "mdbtools.h"

main (int argc, char **argv)
{
int rows;
int i,  j;
unsigned char buf [2048];
MdbHandle *mdb;
MdbCatalogEntry entry;
MdbTableDef *table;
MdbColumn *col;
/* doesn't handle tables > 256 columns.  Can that happen? */
char *bound_values [256]; 
char delimiter [] = ", ";
char quote_text = 1;
int count;
int started;

 if  (argc < 2) 
   {
     fprintf (stderr, "Usage: %s <file> <table>\n", argv [0]);
     exit (1);
   }

 mdb_init();
 mdb = mdb_open (argv [1]);
 
 mdb_read_catalog (mdb, MDB_TABLE);
 
 for  (i = 0; i < mdb->num_catalog; i++) 
   {
     entry = g_array_index (mdb->catalog, MdbCatalogEntry, i);
     if  (!strcmp (entry.object_name, argv [2])) 
       {
	 table = mdb_read_table (&entry);
	 mdb_read_columns (table);
	 mdb_rewind_table (table);
	 
	 for (j = 0; j < table->num_cols; j++) 
	   {
	     bound_values [j] =  (char *) malloc (256);
	     bound_values [j] [0] = '\0';
	     mdb_bind_column (table, j + 1, bound_values [j]);
	   }

	 fprintf (stdout, "/******************************************************************/\n");
	 fprintf (stdout, "/* THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT EDIT IT!!!!!! */\n");
	 fprintf (stdout, "/******************************************************************/\n");
	 fprintf (stdout, "\n");
	 fprintf (stdout, "#include <stdio.h>\n");
	 fprintf (stdout, "#include \"types.h\"\n");
	 fprintf (stdout, "#include \"dump.h\"\n");
	 fprintf (stdout, "\n");
	 fprintf (stdout, "const %s %s_array [] = {\n", argv [2], argv [2]);

	 count = 0;
	 started = 0;
	 while (mdb_fetch_row (table)) 
	   {
	     if (started != 0)
	       {
		 fprintf (stdout, ",\n");
	       }
	     started = 1;
	     fprintf (stdout, "{\t\t\t\t/* %6d */\n\t", count);
	     for  (j = 0; j < table->num_cols; j++) 
	       {
		 fprintf (stdout, "\t");
		 col = g_ptr_array_index (table->columns, j);
		 if  (quote_text && 
		      (col->col_type == MDB_TEXT ||
		       col->col_type == MDB_MEMO)) 
		   {
		     fprintf (stdout, "\"%s\"", bound_values [j]);
		   } 
		 else 
		   {
		     fprintf (stdout, "%s", bound_values [j]);
		   }
		 if (j != table->num_cols - 1)
		   {
		     fprintf (stdout, "%s\n", delimiter);
		   }
		 else
		   {
		     fprintf (stdout, "\n");
		   }
	       }
	     fprintf (stdout, "}");
	     count++;
	   }
	 for  (j = 0; j < table->num_cols; j++) 
	   {
	     free (bound_values [j]);
	   }
       }
   }
 
 mdb_free_handle (mdb);
 mdb_exit();

 fprintf (stdout, "\n};\n");
 fprintf (stdout, "\nconst int %s_array_length = %d;\n", 
	  argv [2],
	  count);

}

