%{
/*
 * Rog-O-Matic
 * Automatically exploring the dungeons of doom.
 *
 * Copyright (C) 2008 by Anthony Molinaro
 * Copyright (C) 1985 by Appel, Jacobson, Hamey, and Mauldin.
 *
 * This file is part of Rog-O-Matic.
 *
 * Rog-O-Matic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rog-O-Matic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rog-O-Matic.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * datesub.l:
 */

%}
%%
"Jan"				printf ("1"); 
"Feb"				printf ("2"); 
"Mar"				printf ("3"); 
"Apr"				printf ("4"); 
"May"				printf ("5"); 
"Jun"				printf ("6"); 
"Jul"				printf ("7"); 
"Aug"				printf ("8"); 
"Sep"				printf ("9"); 
"Oct"				printf ("10"); 
"Nov"				printf ("11"); 
"Dec"				printf ("12"); 
%%

yywrap ()
{
;
}

main ()
{ while (yylex ())
    ; 
}
