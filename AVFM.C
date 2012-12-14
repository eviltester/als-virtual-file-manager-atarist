#include <stdlib.h>
#include <stdio.h>
#include <osbind.h>
#include <vdi.h>
#include	<aes.h>
#include <dos.h>
#include <string.h>
#include "f:\launcher\avfm.h"
#include <time.h>

/*		functions	*/

int unique_from_item(int item);
void copy_file(FILE *in, FILE *out, char *buffer, int buffer_size);
int how_many_in_this_dir(long dir_number);
int max_in_window(int window_handle);
void truncate_path_to(char *,char *,int);
void copy_item(void);
void cut_item(void);
void paste_item(void);
void edit_item(void);
int execute_other(void);
int execute_doc(int item);
void sort_virtual_files( void );
void free_up_the_virtual_files( void );
int item_at(short mousex, short mousey);
void handle_double_click(int item);
void redraw_window( int window_handle, GRECT *dirty);
void display_virtual_file_names(int window_handle, GRECT *clip_area);
void activate_cut_copy(void );
void activate_paste(void );
void delete_highlight(void);
void rationalise_unique_no(int need_unique);
void rationalise_array(void);
void renumber_unique(int old_unique, int new_unique,int old_unique_index);
int how_many_files(void);
int create_directory_name_for(char *dir_name);
int create_what(void);
int create_program_name_for(char *prog_name,char **prog_path, int *args_type, char **args_string,char **working_dir);
int create_document_for(
				char *doc,
				char **doc_path,
				char **doc_comment /*args string*/,
				char **package /*working_dir*/,
				char *execute_by,		/* 'D' for document! 'M' for Multitask, 'H' for Hog, 'E' for exit */
				long *date_created,
				long *current_version,
				long *date_last_edit,
				long *program_unique_no
);
int free_slot_at(short mousex, short mousey);
void add_item( void );
void clearw(int window_handle);
void wclip(int window_handle,GRECT *clip_area);
void fullw(int window_handle);
void remove_mask(char *path_string);
void select_deselect(int item);
void set_bar_size(int window_handle);
void set_bar_pos(int window_handle);
void set_first_item(int window_handle);


/*		version globals	*/

	char	version_number[2];
	int	file_system_version_number;
	char	version_date[9];

/*	user definable options */
	
	#define	YES	1
	#define	NO		2
	#define 	ASK	3

	int 	save_vfs_on_exec;	
	int	save_vfs_on_exit;
	int	backup_vfs;

	int	cls_on_exec_return;	/* ASK not applicable */
	
	

/*		GEM	globals	*/

	/* v_opnvwk input array */
	short work_in[11]={1,1,1,1,1,1,1,1,1,1,2};

	/* v_opnvwk output array */
	short work_out[57];
	short msg[8];
	short	ap_id;

	/*working storage */
	short handle,char_height,char_width,cell_height,cell_width;
	GRECT	temp_grect,temp_grect2;

	int main_window;
	int main_window_attribs;
	short s_dummy;
	short mousex,mousey,button,shift_key,key,clicks;

	/*	prog globals	*/
	char alert_text[80];
	int stop_prog;
	long pexec_error_code;
	char pexec_prog_name[FMSIZE+FNSIZE];
	char tail[127];

	char virtual_file_system_file[FMSIZE+FNSIZE];
	char program_path[FMSIZE];


	typedef struct{
				long parent;			/* 0 if root level */
				long unique_no;
				char *path;				/* exec/doc path or NULL if directory */
				char *display_text;
				int	args_type;		/* 0 - none, 1 - ask, 2 - below */
				char *args;				/* also document comment */
				char *working_dir;	/* also package path or name! */
				char execute_by;		/* 'D' for document! 'M' for Multitask, 'H' for Hog, 'E' for exit */
				long date_created;
				long current_version;
				long date_last_edit;
				long program_unique_no;
			}virtual_file;

	#define DEFAULT_VIRTUAL_FILES 20
	int MAX_VIRTUAL_FILES;

/*	virtual_file virtual_file_system[MAX_VIRTUAL_FILES];
*/
	virtual_file *virtual_file_system;
	virtual_file temp_virtual_file;
	int temp_virtual_file_active;
	
	int current_virtual_files;
	int current_dir_level;			/* the current directory number */
	int unique_number_count;
	int vfs_has_changed;

	#define MAX_DISPLAY_LENGTH				20
	
	#define MIN_WINDOW_WIDTH				50
	#define DEFAULT_COLUMNS_ON_SCREEN	20
	#define DEFAULT_LINES_ON_SCREEN		10
	#define DISPLAY_TEXT_LEN				50
	#define ARG_SIZE							95

	int ret,ret2;		/* generic return code */

	int first_on_screen;		/* number of occurence on screen first (starts at 1)*/
	int highlight;				/* highlight which occurence? 0=none*/
	int number_in_this_dir;	/* how many in this dir?*/


	/* file selector stuff */
	char path[FMSIZE];
	char filename[FNSIZE];


int unique_from_item(int item)
{
	int loop;

	if(virtual_file_system!=NULL)
	{
			for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
			{
				if(virtual_file_system[loop].unique_no!=0 && virtual_file_system[loop].parent==current_dir_level)
				{
					if(item==1)
					{
						/* found it */
						item=loop;
						loop=MAX_VIRTUAL_FILES;
					}
					else
						item--;
				}
			}
		return item;
	}
	else
		return 0;
}

int index_from_unique(int unique)
{
	int loop;

	if(virtual_file_system!=NULL)
	{
			for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
			{
				if(virtual_file_system[loop].unique_no==unique)
				{
					return(loop);
				}
			}
	}
	else
		return 0;
}

void remove_symbolic_links_for(int item)
{
	int loop;

	if(virtual_file_system!=NULL)
	{
			for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
			{
				if(virtual_file_system[loop].execute_by=='D' && virtual_file_system[loop].program_unique_no==item)
				{
					sprintf(virtual_file_system[loop].working_dir,"%s",virtual_file_system[item].path);
					virtual_file_system[loop].program_unique_no=-1;
				}
			}
	}
}

int unique_from_path(char *path)
{
	int loop;
	int item;

/*printf("\n1\n");*/
	if(path==NULL)
		return 0;
/*printf("2\n");*/

	item =-1;

	if(virtual_file_system!=NULL)
	{
/*printf("3\n");*/
			for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
			{
				if(virtual_file_system[loop].unique_no!=0 && virtual_file_system[loop].path!=NULL)
				{
/*printf("%s\n%s\n",path,virtual_file_system[loop].path);*/
					if(!strcmp(path,virtual_file_system[loop].path))
					{
						/* found it */
						item=virtual_file_system[loop].unique_no;
						loop=MAX_VIRTUAL_FILES;
					}
				}
			}
		return item;
	}
	else
		return -1;
}

void file_copy(char *from,char *to)
{
	char buffer[300];
	FILE *in,*out;

	in=fopen(from,"r");
	if(in!=NULL)
	{
		out=fopen(to,"w");
		if(out!=NULL)
		{
			copy_file(in,out,buffer,300);
			fclose(out);
		}	
		fclose(in);
	}
}

void copy_file(FILE *in, FILE *out, char *buffer, int buffer_size)
{
	int n_chars;
	while((n_chars=fread(buffer,sizeof(char),buffer_size,in))>0)
	{
		fwrite(buffer,sizeof(char),n_chars,out);
	}
}


void als_unpack_date(long date,char *buf)
{
	long day,month,year;
	char up[6];
	

	utunpk(date,up);

	day=		/*date	&	0x001F0000*/ (long)up[2];
	month = 	/*date 	& 	0x01E00000*/(long)up[1];
	year = 	/*date 	& 	0xFE000000*/(long)up[0]+1970;


	sprintf(buf,"%ld/%ld/%ld",day,month,year);
	
}

void als_unpack_time(long date,char *buf)
{
	
}


/*
 * copy a string into a TEDINFO structure.
 */
void set_tedinfo(OBJECT *tree,int obj,char *source)
{
	char *dest;
	TEDINFO *tp;

	tp=(TEDINFO *)tree[obj].ob_spec;

	dest=/*((TEDINFO *)tree[obj].ob_spec)*/tp->te_ptext;
	strcpy(dest,source);
}

/*
 * copy the string from a TEDINFO into another string
 */
void get_tedinfo(OBJECT *tree, int obj, char *dest)
{
	char *source;

	source=((TEDINFO *)tree[obj].ob_spec)->te_ptext;	/* extract address */
	strcpy(dest,source);
}

int handle_dialog(OBJECT *dlog,int editnum,int zoom)
{
	short x,y,w,h;
	int but;

	form_center(dlog,&x,&y,&w,&h);


	form_dial(FMD_START,0,0,0,0,x,y,w,h);

if(zoom)
	form_dial(FMD_GROW,x+w/2,y+h/2,0,0,x,y,w,h);


	objc_draw(dlog,0,10,x,y,w,h);


	but=form_do(dlog,editnum);

if(zoom)
	form_dial(FMD_SHRINK,x+w/2,y+h/2,0,0,x,y,w,h);


	form_dial(FMD_FINISH,0,0,0,0,x,y,w,h);
	dlog[but].ob_state&=~SELECTED;	/* de-select exit button */
	return but;
}

void draw_dialog(OBJECT *dlog)
{
	short x,y,w,h;

	form_center(dlog,&x,&y,&w,&h);


	form_dial(FMD_START,0,0,0,0,x,y,w,h);
	objc_draw(dlog,0,10,x,y,w,h);
}

void not_draw_dialog(OBJECT *dlog)
{
	short x,y,w,h;

	form_center(dlog,&x,&y,&w,&h);

	form_dial(FMD_FINISH,0,0,0,0,x,y,w,h);
}

void set_normal_button(OBJECT *tree,int parent,int button)
{
	int b;

	for (b=tree[parent].ob_head; b!=parent; b=tree[b].ob_next)
		{

		if (b==button)
			tree[b].ob_state|=SELECTED;
		}
}

void unset_normal_button(OBJECT *tree,int parent,int button)
{
	int b;

	for (b=tree[parent].ob_head; b!=parent; b=tree[b].ob_next)
		{

		if (b==button)
			tree[b].ob_state&=~SELECTED;
		}
}

void set_button(OBJECT *tree,int parent,int button)
{
	int b;

	for (b=tree[parent].ob_head; b!=parent; b=tree[b].ob_next)
		{

		if (b==button)
			tree[b].ob_state|=SELECTED;
		else
			tree[b].ob_state&=~SELECTED;
		}
}

int get_button(OBJECT *tree,int parent)
{
	int b;

	b=tree[parent].ob_head;
	for (; b!=parent && !(tree[b].ob_state&SELECTED); b=tree[b].ob_next)
		;

	return b;
}

int get_normal_button(OBJECT *tree,int parent,int button)
{
	int b;

	for (b=tree[parent].ob_head; b!=parent; b=tree[b].ob_next)
		{

		if (b==button)
			return(tree[b].ob_state&SELECTED);
		}
}

void open_workstation()
{
	v_opnvwk(work_in,&handle,work_out);

	if(!handle)
	{
		graf_mouse( ARROW , NULL);
		form_alert(1,"[3][GEM cannot allocate a|workstation.|The program must abort.][ OK ]");
		appl_exit();
		exit(-1);
	}
}

void activate_cut_copy(void )
{
	OBJECT *obj;
	int res;

	rsrc_gaddr( R_TREE,Normal_Tos,&obj);

	if(highlight==0)
	{
		res=menu_ienable(obj,MNU_AVFM_Cut,0);		
		res=menu_ienable(obj,MNU_AVFM_Copy,0);		
		res=menu_ienable(obj,MNU_AVFM_Edit_Hi,0);		
	}
	else
	{
		res=menu_ienable(obj,MNU_AVFM_Cut,1);		
		res=menu_ienable(obj,MNU_AVFM_Copy,1);		
		res=menu_ienable(obj,MNU_AVFM_Edit_Hi,1);		
	}

}

void activate_paste(void )
{
	OBJECT *obj;
	int res;

	rsrc_gaddr( R_TREE,Normal_Tos,&obj);

	if(temp_virtual_file_active==0)
	{
		res=menu_ienable(obj,MNU_AVFM_Paste,0);		
	}
	else
	{
		res=menu_ienable(obj,MNU_AVFM_Paste,1);		
	}
}

void save_vfs_on(int vfs_save_var)
{
	int ret_val;

/*	form_alert(1,"[2][before_call_saveX][OK]");
*/			if(vfs_has_changed==1 || vfs_save_var==2)
			{
				if(vfs_save_var==YES || vfs_save_var==2)
				{
					/* save it */
					if(virtual_file_system!=NULL)
					{
/*					form_alert(1,"[2][before_call_save][OK]");
*/						save_virtual_file_system(virtual_file_system_file);
/*					form_alert(1,"[2][before_call_save2][OK]");
*/
/*						free_up_the_virtual_files();
*/
/*					form_alert(1,"[2][before_call_save3][OK]");
*/					}
				}
				else
				{
					if(vfs_save_var==ASK)
					{

						sprintf(alert_text,"[2][The Virtual File System|has changed.| |Do you want to save it?][SAVE|NO]");
						ret_val=form_alert(1,alert_text);
						if(ret_val==1)
						{
							/* save it */
							if(virtual_file_system!=NULL)
							{
	/*				form_alert(1,"[2][before_call_saveA][OK]");
*/
								save_virtual_file_system(virtual_file_system_file);
/*					form_alert(1,"[2][before_call_saveB][OK]");
*/
/*								free_up_the_virtual_files();
*/
/*					form_alert(1,"[2][before_call_saveC][OK]");
*/							}
						}
					}
				}

			}


}

int go_up_a_dir_level()
{

	int loop;

	if(current_dir_level==0)
	{
		ret2=form_alert(1,"[2][ |You are on the top|level directory.| |Do you want to quit?][ YES | NO ]");
		if(ret2==1)
		{
			save_vfs_on(save_vfs_on_exit);
			return 1;
		}
	}
	else
	{
		/* change current_dir_level */
		for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
		{
			if(virtual_file_system[loop].unique_no==current_dir_level)
			{
				ret2=loop;
				loop=MAX_VIRTUAL_FILES;
			}
		}
												
		current_dir_level=virtual_file_system[ret2].parent;
		if(current_dir_level==0)
			loop = wind_set(main_window,WF_NAME,ADDR("ROOT"));
		else
		{
			for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
			{
				if(virtual_file_system[loop].unique_no==current_dir_level)
				{
					ret2=loop;
					loop=MAX_VIRTUAL_FILES;
				}
			}
			loop = wind_set(main_window,WF_NAME,ADDR(virtual_file_system[ret2].display_text));
		}
		first_on_screen=0;		
		highlight=0;			
		number_in_this_dir=how_many_in_this_dir(current_dir_level);
		if(number_in_this_dir!=0)
			first_on_screen=1;
		display_virtual_file_names(main_window,NULL);
		set_bar_size(main_window);
		set_bar_pos(main_window);

	}

	return 0;

}

int how_many_files(void)
{
	int loop;
	int count;

	count=0;
	for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
		if(virtual_file_system[loop].unique_no!=0)
			count++;
	return count;
}

virtual_file *expand_virtual_file_system(int current_size)
{

	void *vp;
	virtual_file *vfp;
	int new_size;
	int loop;


	new_size=current_size+DEFAULT_VIRTUAL_FILES;

	if(virtual_file_system==NULL)
	{
		vp=malloc(sizeof(virtual_file)*new_size);

	}
	else
		{
			vp=realloc(virtual_file_system,sizeof(virtual_file)*new_size);
		}

	if(vp==NULL)
	{
		return virtual_file_system;
	}
	else
	{
		vfp=(virtual_file *)vp;

		for(loop=MAX_VIRTUAL_FILES;loop<new_size;loop++)
		{
			vfp[loop].path=NULL;
			vfp[loop].display_text=NULL;
			vfp[loop].unique_no=0;
			vfp[loop].execute_by=' ';
		}
		MAX_VIRTUAL_FILES=new_size;
		return (virtual_file *)vp;
	} 
	
}

/* return non zero if error */
int save_virtual_file_system(char *out_filename)
{
	FILE *out_file;
	int loop;
	int count;
	char cap[FNSIZE+FMSIZE];
	char *cp;

	if(backup_vfs==YES)
	{
		/* if avfm.vfs && avfm.vfb then delete 
			avfm.vfb
			rename avfm.vfs to avfm.vfb */

			cp=cap;
			strcpy(cap,out_filename);
			cap[strlen(cap)-1]='B';
			file_copy(out_filename,cap);
/*			rename(out_filename,cp);
			cap[strlen(cap)-1]='S';
*/	}

	out_file=fopen(out_filename,"w");

	if(out_file==NULL)
		return 1;


	/* rationalise it */
		rationalise_array();
	/* sort the array */
		sort_virtual_files();

	/* write the file */

	/* write the header */
	fprintf(out_file,"AVFM\n");
	fprintf(out_file,"%c.%c\n",version_number[0],version_number[1]);


/* loop through them all counting them */
	count=0;
	for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
		if(virtual_file_system[loop].unique_no!=0)
			count++;

	fprintf(out_file,"%d\n",count);

	/* loop through them all, writing them */
	for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
		if(virtual_file_system[loop].unique_no!=0)
		{
/*			sprintf(alert_text,"[3][loop %d][ok]",loop);
			form_alert(1,alert_text);
*/
/*form_alert(1,"[2][saving][Parent]");
*/			fprintf(out_file,"%d\n",virtual_file_system[loop].parent);

/*form_alert(1,"[2][saving][unique]");
*/			fprintf(out_file,"%d\n",virtual_file_system[loop].unique_no);

/*form_alert(1,"[2][saving][Path]");
*/			if(virtual_file_system[loop].path==NULL)
				fprintf(out_file,"0\n\n");
			else
			{
				fprintf(out_file,"%d\n",strlen(virtual_file_system[loop].path));
				fprintf(out_file,"%s\n",virtual_file_system[loop].path);
			}

/*form_alert(1,"[2][saving][display_text]");
*/			if(virtual_file_system[loop].display_text==NULL)
				fprintf(out_file,"0\n\n");
			else
			{
				fprintf(out_file,"%d\n",strlen(virtual_file_system[loop].display_text));
				fprintf(out_file,"%s\n",virtual_file_system[loop].display_text);
			}

				fprintf(out_file,"%d\n",virtual_file_system[loop].args_type);

/*form_alert(1,"[2][saving][Args]");
*/			if(virtual_file_system[loop].args==NULL)
				fprintf(out_file,"0\n\n");
			else
			{
				fprintf(out_file,"%d\n",strlen(virtual_file_system[loop].args));
				fprintf(out_file,"%s\n",virtual_file_system[loop].args);
			}

/*form_alert(1,"[2][saving][Working dir]");
*/			if(virtual_file_system[loop].working_dir==NULL)
				fprintf(out_file,"0\n\n");
			else
			{
				fprintf(out_file,"%d\n",strlen(virtual_file_system[loop].working_dir));
				fprintf(out_file,"%s\n",virtual_file_system[loop].working_dir);
			}
/*form_alert(1,"[2][saving][Execute]");
*/		fprintf(out_file,"%c\n",virtual_file_system[loop].execute_by);

		if(virtual_file_system[loop].execute_by=='D')
		{
			fprintf(out_file,"%ld\n",virtual_file_system[loop].date_created);
			fprintf(out_file,"%ld\n",virtual_file_system[loop].current_version);
			fprintf(out_file,"%ld\n",virtual_file_system[loop].date_last_edit);
			fprintf(out_file,"%ld\n",virtual_file_system[loop].program_unique_no);
		}

		}

	fclose(out_file);

	vfs_has_changed=0;

	return 0;
}

void turn_new_line_into_zero(char *cp,int buffer_size)
{
	while(*cp!='\n' && buffer_size!=0)
		{cp++;buffer_size--;}

	*cp='\0';
}

/* return non-zero if error */
int load_virtual_file_system(char *in_fname)
{
	FILE *in_fp;
	char buffer[10];
	int error;

	in_fp=fopen(in_fname,"r");

	if(in_fp==NULL)
		return 1;

	/* OK check if it is an AVFM file */
	fgets(buffer,4,in_fp);
	buffer[4]='\0';	/* NULL terminate it just in case */

	
	if(!strcmp(buffer,"AVFM"))
		/* oops not AVFM file */
		return 2;

	/* seems like an AVFM file, so skip newline */
	fgets(buffer,9,in_fp);

	/* now read the file version type */
	fgets(buffer,9,in_fp);

	turn_new_line_into_zero(buffer,9);


	error=can_we_load_vfs(buffer,in_fp);

	if(error!=0)
	{
			free_up_the_virtual_files();
	}
	else
	{
			/* set the display variables etc */
			current_virtual_files=how_many_files();
			current_dir_level = 0;
			unique_number_count=current_virtual_files+1;
			if(current_virtual_files==0)
				first_on_screen=0;		
			else
				first_on_screen=1;
			highlight=0;			
			number_in_this_dir=how_many_in_this_dir(current_dir_level);
	}

	fclose(in_fp);
	return error;
}

int can_we_load_vfs(char *version_no,FILE *in_fp)
{

	if(!strcmp(version_no,"1.0") ||
		!strcmp(version_no,"1.1"))
	{
		return load_vfs_ver_1_0(in_fp,version_no);
	}
	else
	{
		return 3;
	}
}

int load_vfs_ver_1_0(FILE *infile, char *version_no)
{
	#define IN_BUF_SIZE	300
	char input_buffer[IN_BUF_SIZE];	/* probably a tad extreme!*/
	int files;
	int error;
	int l;
	long ll;
	virtual_file tvf;

	/* get the number of files, just to initialise the size of vfs */

	fgets(input_buffer,IN_BUF_SIZE,infile);
	turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);
	error=stcd_i(input_buffer,&files);

	if(error==0)
		return 4;

	virtual_file_system=expand_virtual_file_system(files);

	if(virtual_file_system==NULL)
		return 5;


		/* read ahead for get int parent */
		fgets(input_buffer,IN_BUF_SIZE,infile);
		turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);


	/* read the files in! */
	while(!feof(infile))
	{

		error=stcd_i(input_buffer,&tvf.parent);
		if(error==0)
			return 6;

		/* get int unique_no */
		fgets(input_buffer,IN_BUF_SIZE,infile);
		turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);

		error=stcd_i(input_buffer,&tvf.unique_no);
		if(error==0)
			return 7;



		/* get char *path; */				/* exec path or NULL if directory */
		fgets(input_buffer,IN_BUF_SIZE,infile);
		turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);

		error=stcd_i(input_buffer,&l);
/*		if(error==0)
			return 8;
*/		
		/* l contains length of path */
		/* if l < FMSIZE+FNSIZE then alloc FMSIZE+FNSIZE */
		/* else alloc space needed */

		if(l==0)
		{
 			tvf.path=NULL;
			fgets(input_buffer,IN_BUF_SIZE,infile);
		}
		else
		{
			if( l < FMSIZE+FNSIZE)
			{
 				tvf.path=(char *)malloc(FMSIZE+FNSIZE);
			}
			else
				tvf.path=(char *)malloc(l+1);
	
			if(tvf.path==NULL)
			{
				free_up_the_virtual_files();
				return 9;
			}

		
			/* get path now */
			fgets(input_buffer,IN_BUF_SIZE,infile);
			turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);
 	
			/* copy path */
			strcpy(tvf.path,input_buffer);
		}

		/* get int display_text size; */
		fgets(input_buffer,IN_BUF_SIZE,infile);
		turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);


		error=stcd_i(input_buffer,&l);
/*		if(error==0)
			return 10;
*/		
		/* l contains length of display_text */
		/* if l < DISPLAY_TEXT_LEN */
		/* else alloc space needed */

		if( l < DISPLAY_TEXT_LEN)
		{
 			tvf.display_text=(char *)malloc(DISPLAY_TEXT_LEN);
		}
		else
			tvf.display_text=(char *)malloc(l+1);

		if(tvf.display_text==NULL)
		{
			free(tvf.path);
			free_up_the_virtual_files();
			return 11;
		}	
			/* get display_text now */
			fgets(input_buffer,IN_BUF_SIZE,infile);
			turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);

			/* copy display_text */
			strcpy(tvf.display_text,input_buffer);

		/* get int args_type */
		fgets(input_buffer,IN_BUF_SIZE,infile);
		turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);
		error=stcd_i(input_buffer,&l);
 		tvf.args_type=l;

/*printf("read args type %d\n",tvf.args_type);
*/		
		/* get int args size; */
		fgets(input_buffer,IN_BUF_SIZE,infile);
		turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);


		error=stcd_i(input_buffer,&l);

/*printf("read args size %d\n",l);
*/
/*		if(error==0)
			return 12;
*/		
		/* l contains length of args */
		/* if l < ARG_SIZE then alloc ARG_SIZE */
		/* else alloc space needed */

		if(l==0)
		{
 			tvf.args=NULL;
			fgets(input_buffer,IN_BUF_SIZE,infile);
		}
		else
		{
			if( l < ARG_SIZE)
			{
 				tvf.args=(char *)malloc(ARG_SIZE);
			}
			else
				tvf.args=(char *)malloc(l+1);

			if(tvf.args==NULL)
			{
				free(tvf.path);
				free(tvf.display_text);
				free_up_the_virtual_files();
				return 13;
			}

			/* get args now */
			fgets(input_buffer,IN_BUF_SIZE,infile);
			turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);

			/* copy args */
			strcpy(tvf.args,input_buffer);
/*printf("read args %s\n",tvf.args);
*/		}

		/* get int working_dir_size; */
		fgets(input_buffer,IN_BUF_SIZE,infile);
		turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);
		
		error=stcd_i(input_buffer,&l);
/*		if(error==0)
			return 14;
*/		
		/* l contains length of working_dir */
		/* if l < FMSIZE+FNSIZE then alloc FMSIZE+FNSIZE */
		/* else alloc space needed */

		if(l==0)
		{
 			tvf.working_dir=NULL;
			fgets(input_buffer,IN_BUF_SIZE,infile);
		}
		else		
		{
			if( l < FMSIZE+FNSIZE)
			{
 				tvf.working_dir=(char *)malloc(FMSIZE+FNSIZE);
			}
			else
				tvf.working_dir=(char *)malloc(l+1);
	
			if(tvf.working_dir==NULL)
			{
				free(tvf.path);
				free(tvf.display_text);
				free(tvf.args);
				free_up_the_virtual_files();
				return 15;
			}


			/* get working dir now */
			fgets(input_buffer,IN_BUF_SIZE,infile);
			turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);
	
			/* copy path */
			strcpy(tvf.working_dir,input_buffer);
/*printf("read working dir %s\n",tvf.working_dir);
*/		}

		/* get char execute_by; */		/* 'M' for Multitask, 'H' for Hog, 'E' for exit */
		fgets(input_buffer,IN_BUF_SIZE,infile);
		turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);
		tvf.execute_by=input_buffer[0];
	
		if(tvf.execute_by!='M' &&
			tvf.execute_by!='H' &&
			tvf.execute_by!='E' &&
			tvf.execute_by!='D')
			tvf.execute_by=' ';

	if(!strcmp(version_no,"1.1") && tvf.execute_by=='D')
	{
/*form_alert(1,"[1][loadin' v1.1 for doc][ok]");
*/
		/*	long date_created;	*/
		fgets(input_buffer,IN_BUF_SIZE,infile);
		turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);
		error=stcd_l(input_buffer,&ll);
		tvf.date_created=ll;

		/*	long current_version;*/
		fgets(input_buffer,IN_BUF_SIZE,infile);
		turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);
		error=stcd_l(input_buffer,&ll);
		tvf.current_version=ll;

		/*	long date_last_edit;*/
		fgets(input_buffer,IN_BUF_SIZE,infile);
		turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);
		error=stcd_l(input_buffer,&ll);
		tvf.date_last_edit=ll;

		/* long program_unique_no;*/
		fgets(input_buffer,IN_BUF_SIZE,infile);
		turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);
		error=stcd_l(input_buffer,&ll);
		tvf.program_unique_no=ll;
/*sprintf(alert_text,"[1][loadin' v1.1 for doc|prog unique = %ld|buf was %s, ll = %ld][ok]",tvf.program_unique_no,input_buffer,ll);

form_alert(1,alert_text);
*/
	}


		add_item_given_details(tvf.parent,tvf.unique_no,tvf.path,tvf.display_text,tvf.args_type,tvf.args,tvf.working_dir,tvf.execute_by,tvf.date_created,tvf.current_version,tvf.date_last_edit,tvf.program_unique_no);		

		/* read ahead for get int parent */
		fgets(input_buffer,IN_BUF_SIZE,infile);
		turn_new_line_into_zero(input_buffer,IN_BUF_SIZE);


	}	

return 0;
	
}


	



int main(int argc, char *argv[]){

	int loop;
	int oh;
	int curr_drive; 
	int error;
	/*char key_num_str[40];*/
	short short_dummy,win_slide_size,win_slide_pos;
	OBJECT *obj;

	/*		set up the version details stuff	*/
	version_number[0]='1';
	version_number[1]='1';
	file_system_version_number=1;
	strcpy(version_date,"25/05/95");
	
	MAX_VIRTUAL_FILES=0;
	virtual_file_system=NULL;
	vfs_has_changed=0;
	temp_virtual_file_active=0;

	/*		initialise GEM		*/

	/* start AES */
	ap_id = appl_init();
	if(ap_id==-1)
		return -1;		

	/* find AES handle */
	handle=graf_handle(	&char_height,&char_width,
					   		&cell_height,&cell_width);	

	/* open workstation */

	open_workstation();

	stop_prog=0;


	if(!rsrc_load("avfm.rsc"))
	{
		graf_mouse(ARROW,NULL);
		form_alert(1,"[3][The Program Cannot load|resource file AVFM.RSC|In the Program Directory][OK]");
		v_clsvwk(handle);
		appl_exit();
		return -1;
	}
	

	/* set up the about box */

	rsrc_gaddr( R_TREE,About_AVFM,&obj);
	sprintf(alert_text,"%c.%c",version_number[0],version_number[1]);
	set_tedinfo(obj,About_Ver_Text,alert_text);

	sprintf(alert_text,"%s",version_date);
	set_tedinfo(obj,About_Ver_Date,alert_text);



	rsrc_gaddr( R_TREE,Normal_Tos,&obj);
	menu_bar(obj,1);


	/*		Locate config file*/

		/* find out the current path to look for program files */
		curr_drive=getdsk();
		error=getcd(curr_drive+1,program_path);


		save_vfs_on_exit=ASK;
		save_vfs_on_exec=ASK;
		backup_vfs=YES;

	/*		Locate config file*/
	/*			if file exists then load it	*/
		

			
	/*		Locate directory structure file	*/
		sprintf(virtual_file_system_file,"%s\\AVFM.VFS",program_path);

	/*			if file exists then load it	*/
	error=load_virtual_file_system(virtual_file_system_file);

	if(error!=0)
	{
/*		sprintf(alert_text,"[2][file load error %d][OK]",error);
		form_alert(1,alert_text);
*/	}

	/*			otherwise setup default blank directory	*/
	if(virtual_file_system==NULL)
	{

		if(virtual_file_system==NULL)
			virtual_file_system=expand_virtual_file_system(MAX_VIRTUAL_FILES);

		if(virtual_file_system==NULL)
		{
			rsrc_free();
			graf_mouse( ARROW , NULL);
			form_alert(1,"[3][No Memory for Virtual|file system.|The program must abort.][ OK ]");
			v_clsvwk(handle);
			appl_exit();
			return -1;
		}


		for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
		{
			virtual_file_system[loop].path=NULL;
			virtual_file_system[loop].display_text=NULL;
			virtual_file_system[loop].unique_no=0;
		}
			current_virtual_files=how_many_files();
			unique_number_count=current_virtual_files+1;
			first_on_screen=0;		
			highlight=0;			
/*		current_virtual_files=0;
*/
		current_dir_level = 0;
/*		unique_number_count=1;
*/		first_on_screen=0;		
		highlight=0;			
/*		number_in_this_dir=0;
*/			number_in_this_dir=how_many_in_this_dir(current_dir_level);
		if(number_in_this_dir==0)
			first_on_screen=0;		
		else
			first_on_screen=1;		

	}

	/* start prog */
	graf_mouse( ARROW , NULL);


	/*		create the window and display the root	*/

		main_window_attribs=NAME|CLOSE|FULL|MOVE|SIZE|UPARROW|DNARROW|VSLIDE;
		/* set the window's full state */
		/* by getting the desktop size */
		wind_get(DESK,WF_WXYWH,	&temp_grect.g_x,
										&temp_grect.g_y,
										&temp_grect.g_w,
										&temp_grect.g_h);

		/* set the default full position */
		main_window = wind_create(	main_window_attribs,
											temp_grect.g_x,
											temp_grect.g_y,
											temp_grect.g_w,
											temp_grect.g_h);

		if(main_window<0)
		{
			rsrc_free();
			if(virtual_file_system!=NULL)
				free_up_the_virtual_files();

			form_alert(1,"[3][GEM cannot allocate |any more windows.|The program must abort.][ OK ]");
			v_clsvwk(handle);
			appl_exit();
			return -1;
		}

		ret = wind_set(main_window,WF_NAME,ADDR("ROOT"));
		wind_get(DESK,WF_WXYWH,	&temp_grect.g_x,
										&temp_grect.g_y,
										&temp_grect.g_w,
										&temp_grect.g_h);
		if(DEFAULT_COLUMNS_ON_SCREEN*cell_width<temp_grect.g_w)
			temp_grect.g_w=DEFAULT_COLUMNS_ON_SCREEN*cell_width;

		if(DEFAULT_LINES_ON_SCREEN*cell_height<temp_grect.g_h)
			temp_grect.g_h=DEFAULT_LINES_ON_SCREEN*cell_height;
		ret = wind_open(main_window,temp_grect.g_x,temp_grect.g_y,temp_grect.g_w,temp_grect.g_h);

		display_virtual_file_names(main_window,NULL);
		set_bar_size(main_window);
		set_bar_pos(main_window);

		activate_paste();
		activate_cut_copy();

	/*		main loop code	*/
			
		while(!stop_prog)
		{

			ret=evnt_multi(MU_MESAG|MU_KEYBD|MU_BUTTON,2,1,1,0,0,0,0,0,0,0,0,0,0,msg,0,0,&mousex,&mousey,&button,&shift_key,&key,&clicks);

			if(ret & MU_MESAG)
			{
				switch(msg[0])
				{
					case MN_SELECTED:
												rsrc_gaddr( R_TREE,Normal_Tos,&obj);
												menu_tnormal(obj,msg[3],1);
										switch(msg[4])
										{
											case MNU_AVFM_About:
												rsrc_gaddr( R_TREE,About_AVFM,&obj);
												error=handle_dialog(obj,0,0);
/*												form_alert(1,"[3][Al's|Virtual|File|Manager][ :O ]");
*/												break;
											case MNU_AVFM_Save:
												save_vfs_on(2);
												break;
											case MNU_AVFM_Other:
												save_vfs_on(save_vfs_on_exec);
												execute_other();
												break;
											case MNU_AVFM_Quit:
												save_vfs_on(save_vfs_on_exit);
												stop_prog=1;
												break;												
											case MNU_AVFM_Cut:
												cut_item();
												highlight=0;
												display_virtual_file_names(main_window,NULL);
												set_bar_size(main_window);
												set_bar_pos(main_window);
												break;											
											case MNU_AVFM_Copy:
												copy_item();
												highlight=0;
												display_virtual_file_names(main_window,NULL);
												set_bar_size(main_window);
												set_bar_pos(main_window);
												break;											
											case MNU_AVFM_Paste:
												paste_item();
												highlight=0;
												display_virtual_file_names(main_window,NULL);
												set_bar_size(main_window);
												set_bar_pos(main_window);
												break;											
											case MNU_AVFM_Edit_Hi:
												edit_item();
												/*highlight=0;*/
												display_virtual_file_names(main_window,NULL);
												set_bar_size(main_window);
												set_bar_pos(main_window);
												break;
											case MNU_AVFM_Add_New:
												add_item();
												highlight=0;
												display_virtual_file_names(main_window,NULL);
												set_bar_size(main_window);
												set_bar_pos(main_window);
												break;
										}

										break;
					case WM_FULLED:
										fullw(main_window);
										set_bar_size(main_window);
										set_bar_pos(main_window);
										break;
					case WM_REDRAW:
										redraw_window(msg[3], (GRECT *)&msg[4]);
										break;
					case WM_CLOSED:
											stop_prog=go_up_a_dir_level();
											set_bar_size(main_window);
											set_bar_pos(main_window);
											break;
					case WM_SIZED:
					case WM_MOVED:
											if( msg[6]<MIN_WINDOW_WIDTH)
												msg[6]=MIN_WINDOW_WIDTH;
											wind_set(msg[3],WF_CXYWH,msg[4],msg[5],msg[6],msg[7]);
											set_bar_size(main_window);
											set_bar_pos(main_window);
											set_first_item(main_window);
											display_virtual_file_names(main_window,NULL);
											break;
					case WM_VSLID:
											wind_set(msg[3],WF_VSLIDE,msg[4]);
											set_first_item(main_window);
											display_virtual_file_names(main_window,NULL);
											break;
					case WM_ARROWED:
									switch(msg[4])
									{
											case WA_UPPAGE:
																wind_get(msg[3],WF_VSLIDE,&win_slide_pos,&short_dummy,&short_dummy,&short_dummy);
																wind_get(msg[3],WF_VSLSIZE,&win_slide_size,&short_dummy,&short_dummy,&short_dummy);
/*sprintf(alert_text,"[1][pos = %d|size = %d|new pos %d][ok]",win_slide_pos,win_slide_size,win_slide_pos-win_slide_size);
form_alert(1,alert_text);
*/																short_dummy=(win_slide_pos-win_slide_size)-win_slide_size;
																wind_set(msg[3],WF_VSLIDE,short_dummy);
																set_first_item(main_window);
																display_virtual_file_names(main_window,NULL);
																break;
											case WA_DNPAGE:
																wind_get(msg[3],WF_VSLIDE,&win_slide_pos,&short_dummy,&short_dummy,&short_dummy);
																wind_get(msg[3],WF_VSLSIZE,&win_slide_size,&short_dummy,&short_dummy,&short_dummy);
																short_dummy=(win_slide_pos+win_slide_size)+win_slide_size;
																wind_set(msg[3],WF_VSLIDE,short_dummy);
																set_first_item(main_window);
																display_virtual_file_names(main_window,NULL);
																break;
									}
									break;

				}
			}

			if(ret & MU_KEYBD)
			{
				switch(key )
				{
					case 0x5200:	/* insert */
									add_item();
									highlight=0;
									display_virtual_file_names(main_window,NULL);
									set_bar_size(main_window);
									set_bar_pos(main_window);
									break;
					case 0x2E03:	/* control C */
									copy_item();
									highlight=0;
									display_virtual_file_names(main_window,NULL);
									set_bar_size(main_window);
									set_bar_pos(main_window);
									break;
					case 0x2D18:	/* control X */
									cut_item();
									highlight=0;
									display_virtual_file_names(main_window,NULL);
									set_bar_size(main_window);
									set_bar_pos(main_window);
									break;
					case 0x2F16:	/* control V */
									paste_item();
									highlight=0;
									display_virtual_file_names(main_window,NULL);
									set_bar_size(main_window);
									set_bar_pos(main_window);
									break;
					case 0x6100:	/* UNDO */
									edit_item();
									/*highlight=0;*/
									display_virtual_file_names(main_window,NULL);
									set_bar_size(main_window);
									set_bar_pos(main_window);
									break;
					case 0x537F:	/* delete */
									delete_highlight();
									highlight=0;
									display_virtual_file_names(main_window,NULL);
									set_bar_size(main_window);
									set_bar_pos(main_window);
									break;
					case 0x5000:	/* down */
									if(highlight==0)
									{
										highlight=first_on_screen;
										display_virtual_file_names(main_window,NULL);
										set_bar_size(main_window);
										set_bar_pos(main_window);
									}
									else
									{
										if(highlight!=number_in_this_dir)
										{
											highlight=highlight+1;
											if(highlight>=first_on_screen+max_in_window(main_window))
												first_on_screen++;
											display_virtual_file_names(main_window,NULL);
											set_bar_size(main_window);
											set_bar_pos(main_window);
										}		
									}
									break;
					case 0x4800:	/* up */
									if( highlight==0)
									{
										/*highlight=first_on_screen;*/
										select_deselect(first_on_screen);
/*										display_virtual_file_names(main_window,NULL);
*/
										set_bar_size(main_window);
										set_bar_pos(main_window);

									}
									else
									{
										if(highlight!=1)
										{
										
											oh=highlight;
											select_deselect(highlight);
											if(oh-1<first_on_screen)
												first_on_screen=oh-1;
											select_deselect(oh-1);							
											set_bar_size(main_window);
											set_bar_pos(main_window);
										}		
									}
									break;
					case 0x1C0D:	/* enter */
									if(highlight!=0)
									{
										handle_double_click(highlight);
										set_bar_size(main_window);
										set_bar_pos(main_window);
									}
									break;
					case 0x11B:	/* esc */
									stop_prog=go_up_a_dir_level();
									set_bar_size(main_window);
									set_bar_pos(main_window);
									break;

				
					default:
/*								sprintf(key_num_str,"[0][ %X ][OK]",key);
								form_alert(1,key_num_str);
*/								break;
				}
			}

			if(ret & MU_BUTTON)
			{
				switch(clicks)
				{	
					case 1:	/* select/deselect */
							if(button & 0x02)			/* right down too */
							{
								oh=item_at(mousex,mousey);
								if(oh!=0)
								{
									select_deselect(highlight);
									select_deselect(oh);
									edit_item();
									/*highlight=0;*/
									display_virtual_file_names(main_window,NULL);
									set_bar_size(main_window);
									set_bar_pos(main_window);
								}
							}
							else
							{
								oh=item_at(mousex,mousey);
								if(oh!=0)
								{
									select_deselect(highlight);
									select_deselect(oh);
								}
							}
								break;

					case 2:	/* execute or change dir */
							if(button & 0x02)			/* right down too */
							{
								oh=item_at(mousex,mousey);
								if(oh!=0)
								{
									select_deselect(highlight);
									select_deselect(oh);
									edit_item();
									/*highlight=0;*/
									display_virtual_file_names(main_window,NULL);
									set_bar_size(main_window);
									set_bar_pos(main_window);
								}
							}
							else
							{
								oh=item_at(mousex,mousey);
								if(oh!=0)	
								{
									handle_double_click(oh);
									set_bar_size(main_window);
									set_bar_pos(main_window);
								}
								else
								{
									oh=free_slot_at(mousex,mousey);
									if(oh!=0)
									{
										add_item();
										highlight=0;
										display_virtual_file_names(main_window,NULL);
										set_bar_size(main_window);
										set_bar_pos(main_window);
									}
								}
							}
								break;
				}
				
			}

			activate_paste();
			activate_cut_copy();

		}


	free_up_the_virtual_files();
	rsrc_free();
	menu_bar(obj,0);
	wind_close(main_window);
	wind_delete(main_window);
	v_clsvwk(handle);
	appl_exit();
	return 0;

}	 

void truncate_path_to(char *path_text,char *final_text,int final_text_len)
{
	int front,middle,back;
	int loop,cnt;


	if(path_text==NULL)
	{
		final_text=NULL;
		goto end_func;
	}

	if(final_text==NULL)
		goto end_func;

	if(strlen(path_text)<final_text_len)
		strcpy(final_text,path_text);
	else
	{

		middle=3;
		front=(final_text_len-3-12)/2;
		back=final_text_len-front-middle;

		cnt=0;
		for(loop=0;loop<front;loop++)
		{
			final_text[cnt]=path_text[loop];
			cnt++;
		}

		for(loop=0;loop<middle;loop++)
		{
			final_text[cnt]='.';
			cnt++;
		}

		for(loop=back;loop>0;loop--)
		{
			final_text[cnt]=path_text[strlen(path_text)-loop];
			cnt++;
		}

		final_text[cnt]='\0';
	}
	


end_func:;
}


int item_at(short mousex, short mousey)
{
	short x,y,w,h,tattributes[10];
	int item;

 
	/* get the text attributes */
	vqt_attributes(handle,tattributes);

	/* cell height is in tattributes[9] */

	/*get the window work area */
	wind_get(main_window,WF_WXYWH,&x,&y,&w,&h);

	if( mousex>=x && mousex<=x+w && mousey<=y+h && mousey >=y)
  	{
		item= mousey-y;
		item = item /tattributes[9];	

		item=item+first_on_screen;

		if(item<=number_in_this_dir)
		{
			return item;
		}
	}

	return 0;
}

int free_slot_at(short mousex, short mousey)
{
	short x,y,w,h;
 
	/*get the window work area */
	wind_get(main_window,WF_WXYWH,&x,&y,&w,&h);

	if( mousex>=x && mousex<=x+w && mousey<=y+h && mousey >=y)
  	{
		return 1;
	}

	return 0;
}

void parse_args_into(char *args_text, char *into, int max_length_args)
{
	char parse_pathname[FNSIZE+FMSIZE];
	char parse_filename[FMSIZE];
	char *cp;
	char *cp2;
	int fs_result;
	short but;

	cp=args_text;
	cp2=into;
	cp2++;

	parse_pathname[0]='\0';
	parse_filename[0]='\0';

	if(cp!=NULL)
	{
		while(*cp!='\0' && max_length_args>0)
		{
/*printf("%c",*cp);
*/			switch(*cp)
			{
				case '%':
							cp++;
							
							switch(*cp)
							{
								case 'f':
								case 'F':
								case 'p':
								case 'P':
											fs_result=fsel_exinput(parse_pathname,parse_filename,&but,"Argument file");
											if(but!=0)
											{
												switch(*cp)
												{
													case 'f':
														if(max_length_args-strlen(parse_filename)>0)
														{
															sprintf(cp2,"%s",parse_filename);
															max_length_args-=strlen(parse_filename);
															cp2+=strlen(parse_filename);
														}
														break;

													case 'F':
														remove_mask(parse_pathname);
														strcat(parse_pathname,parse_filename);
														if(max_length_args-strlen(parse_pathname)>0)
														{
															sprintf(cp2,"%s",parse_pathname);
															max_length_args-=strlen(parse_pathname);
															cp2+=strlen(parse_pathname);
														}
														break;

													case 'p':
														remove_mask(parse_pathname);
														if(max_length_args-strlen(parse_pathname)>0)
														{
															sprintf(cp2,"%s",parse_pathname);
															max_length_args-=strlen(parse_pathname);
															cp2+=strlen(parse_pathname);
														}
														break;

													case 'P':
														if(max_length_args-strlen(parse_pathname)>0)
														{
															sprintf(cp2,"%s",parse_pathname);
															max_length_args-=strlen(parse_pathname);
															cp2+=strlen(parse_pathname);
														}
														break;

												}
											}
											cp++;
											break;
								case '%':
											*cp2='%';
											cp2++;
											cp++;
											max_length_args--;							
											break;
								default:
											cp++;
											break;
							}

							break;
				default:
							*cp2=*cp;
							cp2++;
							cp++;
							max_length_args--;
							break;
			}
		}
/*printf("%s\n",into);
*/		cp=into;
		cp++;
/*printf("%s\n",cp);
*/		*into=(char)strlen(cp);
	}
	else
	{
/*printf("cp==NULL");
*/		into[0]=0;
		into[1]='\0';
	}
}

int execute_program(int item)
{
	int alert_error;
	OBJECT *obj;
	char name_buf[31];
	char *cp;
	char temp_args[91];


/*form_alert(1,"[1][exec][ok]");
*/
	if(virtual_file_system[item].execute_by=='D')
	{
		return execute_doc(item);
	}

	if(virtual_file_system[item].path!=NULL)
	{
/*form_alert(1,"[1][got a path][ok]");
*/		/* change dir */
		/* if there is a working dir then change to it */
		/*		if that is not valid then ask to use prog dir */
		/* if no working dir then change to prog one */

		if(virtual_file_system[item].working_dir!=NULL)
		{
/*form_alert(1,"[1][changing dir][ok]");
*/			sprintf(pexec_prog_name,"%s",virtual_file_system[item].working_dir);
/*sprintf(alert_text,"[1][to %s][ok]",virtual_file_system[item].working_dir);
form_alert(1,alert_text);
*/
			pexec_error_code = chdir(pexec_prog_name);
/*sprintf(alert_text,"[1][error was %d][ok]",pexec_error_code);
form_alert(1,alert_text);
*/
			if(pexec_error_code!=0)
			{
				alert_error=form_alert(1,"[3][The Working dir does not exist|Should I use the program's dir|Or simply not change dir at all?][Program|No Change|Cancel]");
				if(alert_error!=3)
				{
					if(alert_error==1)	/* change to prog */
					{
						sprintf(pexec_prog_name,"%s",virtual_file_system[item].path);
						remove_mask(pexec_prog_name);
						pexec_error_code = chdir(pexec_prog_name);
						if(pexec_error_code!=0)
						{
							sprintf(alert_text,"[3][Path does not exist][OK]");
							alert_error=form_alert(1,alert_text);
						}
					}
				}
				else return 1;
			}
/*form_alert(1,"[1][changed dir][ok]");
*/
		}
		else
		{
/*form_alert(1,"[1][no path][ok]");
*/			sprintf(pexec_prog_name,"%s",virtual_file_system[item].path);
			remove_mask(pexec_prog_name);
			pexec_error_code = chdir(pexec_prog_name);
			if(pexec_error_code!=0)
			{
				sprintf(alert_text,"[3][Path does not exist][OK]");
				form_alert(1,alert_text);
			}
		}


		/* ok now execute it */
/*form_alert(1,"[1][now exec][ok]");
*/
		/* execute a prog */

		if(virtual_file_system[item].args_type==0)
		{
			tail[0]=0;
			tail[1]='\0';
		}
		else
		{
			if(virtual_file_system[item].args_type==1)
			{
				/* ask */
				rsrc_gaddr(R_TREE,FORM_Ask_For_Arg,&obj);
				sprintf(name_buf,virtual_file_system[item].display_text);
				set_tedinfo(obj,Ask_Prog_Name,name_buf);
				if(virtual_file_system[item].args!=NULL)
				{
					cp=virtual_file_system[item].args;
					stccpy(name_buf,cp,30);
					cp+=30;
					set_tedinfo(obj,Ask_Args1,name_buf);

					if(strlen(virtual_file_system[item].args)>30)
						stccpy(name_buf,cp,30);
					else
						name_buf[0]='\0';
					set_tedinfo(obj,Ask_Args2,name_buf);
					cp+=30;


					if(strlen(virtual_file_system[item].args)>30)
						stccpy(name_buf,cp,30);
					else
						name_buf[0]='\0';
					set_tedinfo(obj,Ask_Args3,name_buf);
				}
				else
				{
					set_tedinfo(obj,Ask_Args1,"");
					set_tedinfo(obj,Ask_Args2,"");
					set_tedinfo(obj,Ask_Args3,"");
				}
					
					alert_error=handle_dialog(obj,0,0);

					if(alert_error==Ask_Args_CANCEL)
						return 2;

					get_tedinfo(obj,Ask_Args1,name_buf);
					sprintf(temp_args,"%s",name_buf);
					get_tedinfo(obj,Ask_Args2,name_buf);
					strcat(temp_args,name_buf);
					get_tedinfo(obj,Ask_Args3,name_buf);
					strcat(temp_args,name_buf);
		
					parse_args_into(temp_args,tail,127);
			}
			else
				if(virtual_file_system[item].args_type==2)
				{
					if(virtual_file_system[item].args==NULL)
					{
						/* ask */
						tail[0]=0;
						tail[1]='\0';
					}
					else
					{
						parse_args_into(virtual_file_system[item].args,tail,127);

/*sprintf(alert_text,"[1][%s][tail]",tail);
form_alert(1,alert_text);
*/
/*						tail[0]=(char)strlen(virtual_file_system[item].args);
						strcpy(&tail[1],virtual_file_system[item].args);
*/					}
				}
		}

/*sprintf(alert_text,"[1][%s][tail]",tail);
form_alert(1,alert_text);
*/

		/* normal tos execution */

		wind_get(main_window,WF_CXYWH,	&temp_grect.g_x,
													&temp_grect.g_y,
													&temp_grect.g_w,
													&temp_grect.g_h);

		wind_close(main_window);
		wind_delete(main_window);
		rsrc_gaddr( R_TREE,Normal_Tos,&obj);
		menu_bar(obj,0);
/*		v_clrwk(handle);*/
		v_clsvwk(handle);



		sprintf(pexec_prog_name,"%s",virtual_file_system[item].path);
		pexec_error_code = Pexec(0,pexec_prog_name,tail,NULL);

/*					sprintf(alert_text,"[3][pexec return %ld][OK]",pexec_error_code);
					form_alert(1,alert_text);
					sprintf(alert_text,"[3][for %s][OK]",pexec_prog_name);
					form_alert(1,alert_text);
*/
		/* now back to this prog so tidy the screen etc */
		open_workstation();
		/*v_clrwk(handle);*/

		rsrc_gaddr( R_TREE,Normal_Tos,&obj);
		menu_bar(obj,1);

		/* recreate the window */
		main_window_attribs=NAME|CLOSE|FULL|MOVE|SIZE|UPARROW|DNARROW|VSLIDE;

		/* set the window's full state */
		/* by getting the desktop size */
		wind_get(DESK,WF_WXYWH,	&temp_grect.g_x,
										&temp_grect.g_y,
										&temp_grect.g_w,
										&temp_grect.g_h);

		/* set the default full position */
		main_window = wind_create(	main_window_attribs,
											temp_grect.g_x,
											temp_grect.g_y,
											temp_grect.g_w,
											temp_grect.g_h);

		graf_mouse(ARROW,NULL);

		if(main_window<0)
		{
			form_alert(1,"[3][GEM cannot allocate |any more windows.|The program must abort.][ OK ]");
			v_clsvwk(handle);
			appl_exit();
			exit(-1);
		}

		ret = wind_set(main_window,WF_NAME,ADDR("ROOT"));
				wind_get(DESK,WF_WXYWH,	&temp_grect.g_x,
												&temp_grect.g_y,
												&temp_grect.g_w,
												&temp_grect.g_h);
		if(DEFAULT_COLUMNS_ON_SCREEN*cell_width<temp_grect.g_w)
			temp_grect.g_w=DEFAULT_COLUMNS_ON_SCREEN*cell_width;

		if(DEFAULT_LINES_ON_SCREEN*cell_height<temp_grect.g_h)
			temp_grect.g_h=DEFAULT_LINES_ON_SCREEN*cell_height;

		ret = wind_open(main_window,temp_grect.g_x,temp_grect.g_y,temp_grect.g_w,temp_grect.g_h);
/*		ret = wind_open(main_window,temp_grect.g_x,temp_grect.g_y,DEFAULT_COLUMNS_ON_SCREEN*cell_width,DEFAULT_LINES_ON_SCREEN*cell_height);
*/		display_virtual_file_names(main_window,NULL);
		set_bar_size(main_window);
		set_bar_pos(main_window);
	}
	else
		form_alert(1,"[2][There is no path for this program][!]");	

	return 0;
}

int execute_doc(int item)
{
	int alert_error;
	OBJECT *obj;
/*	char name_buf[31];*/
/*	char *cp;
	char temp_args[91];
*/

/*form_alert(1,"[1][exec][ok]");
*/

	if(virtual_file_system[item].working_dir!=NULL)
	{
/*form_alert(1,"[1][got a path][ok]");
*/		/* change dir */
		/* if there is a working dir then change to it */
		/*		if that is not valid then ask to use prog dir */
		/* if no working dir then change to prog one */

		if(virtual_file_system[item].program_unique_no!=-1)
		{
			sprintf(pexec_prog_name,"%s",virtual_file_system[index_from_unique(virtual_file_system[item].program_unique_no)].path);
		}
		else
		{
			sprintf(pexec_prog_name,"%s",virtual_file_system[item].working_dir);
		}

		remove_mask(pexec_prog_name);
		pexec_error_code = chdir(pexec_prog_name);
		if(pexec_error_code!=0)
		{
				sprintf(alert_text,"[3][Program Path does not exist][OK]");
				alert_error=form_alert(1,alert_text);
		}

/*form_alert(1,"[1][changed dir][ok]");
*/

		/* ok now execute it */
/*form_alert(1,"[1][now exec][ok]");
*/
		/* execute a prog */

		sprintf(&tail[1],"%s",virtual_file_system[item].path);

/*sprintf(alert_text,"[1][%s][tail]",tail);
form_alert(1,alert_text);
*/

		tail[0]=(char)strlen(virtual_file_system[item].path);

/*						strcpy(&tail[1],virtual_file_system[item].args);
*/	


/*sprintf(alert_text,"[1][%s][tail]",tail);
form_alert(1,alert_text);
*/

		/* normal tos execution */

		wind_get(main_window,WF_CXYWH,	&temp_grect.g_x,
													&temp_grect.g_y,
													&temp_grect.g_w,
													&temp_grect.g_h);

		wind_close(main_window);
		wind_delete(main_window);
		rsrc_gaddr( R_TREE,Normal_Tos,&obj);
		menu_bar(obj,0);
/*		v_clrwk(handle);*/
		v_clsvwk(handle);



	if(virtual_file_system[item].program_unique_no!=-1)
	{
		sprintf(pexec_prog_name,"%s",virtual_file_system[index_from_unique(virtual_file_system[item].program_unique_no)].path);
	}
	else
	{
		sprintf(pexec_prog_name,"%s",virtual_file_system[item].working_dir);
	}

		pexec_error_code = Pexec(0,pexec_prog_name,tail,NULL);

/*					sprintf(alert_text,"[3][pexec return %ld][OK]",pexec_error_code);
					form_alert(1,alert_text);
					sprintf(alert_text,"[3][for %s][OK]",pexec_prog_name);
					form_alert(1,alert_text);
*/
		/* now back to this prog so tidy the screen etc */
		open_workstation();
		/*v_clrwk(handle);*/

		rsrc_gaddr( R_TREE,Normal_Tos,&obj);
		menu_bar(obj,1);

		/* recreate the window */
		main_window_attribs=NAME|CLOSE|FULL|MOVE|SIZE|UPARROW|DNARROW|VSLIDE;

		/* set the window's full state */
		/* by getting the desktop size */
		wind_get(DESK,WF_WXYWH,	&temp_grect.g_x,
										&temp_grect.g_y,
										&temp_grect.g_w,
										&temp_grect.g_h);

		/* set the default full position */
		main_window = wind_create(	main_window_attribs,
											temp_grect.g_x,
											temp_grect.g_y,
											temp_grect.g_w,
											temp_grect.g_h);

		graf_mouse(ARROW,NULL);

		if(main_window<0)
		{
			form_alert(1,"[3][GEM cannot allocate |any more windows.|The program must abort.][ OK ]");
			v_clsvwk(handle);
			appl_exit();
			exit(-1);
		}

		ret = wind_set(main_window,WF_NAME,ADDR("ROOT"));
				wind_get(DESK,WF_WXYWH,	&temp_grect.g_x,
												&temp_grect.g_y,
												&temp_grect.g_w,
												&temp_grect.g_h);
		if(DEFAULT_COLUMNS_ON_SCREEN*cell_width<temp_grect.g_w)
			temp_grect.g_w=DEFAULT_COLUMNS_ON_SCREEN*cell_width;

		if(DEFAULT_LINES_ON_SCREEN*cell_height<temp_grect.g_h)
			temp_grect.g_h=DEFAULT_LINES_ON_SCREEN*cell_height;

		ret = wind_open(main_window,temp_grect.g_x,temp_grect.g_y,temp_grect.g_w,temp_grect.g_h);
/*		ret = wind_open(main_window,temp_grect.g_x,temp_grect.g_y,DEFAULT_COLUMNS_ON_SCREEN*cell_width,DEFAULT_LINES_ON_SCREEN*cell_height);
*/		display_virtual_file_names(main_window,NULL);
		set_bar_size(main_window);
		set_bar_pos(main_window);

		virtual_file_system[item].date_last_edit=time(NULL); 
		vfs_has_changed=1;

		return 1;

	}
	else
		form_alert(1,"[2][There is no path for this document][!]");	

	return 0;
}

int execute_other()
{
	int alert_error;
	int fs_ret;
	OBJECT *obj;
	char name_buf[31];
	/*char *cp;*/
	char path[FNSIZE+FMSIZE];
	char temp_args[91];
	short but;

	path[0]='\0';
	name_buf[0]='\0';

	fs_ret=fsel_exinput(path,name_buf,&but,"CHOOSE PROG");
	if(but==0)
		return 0;

	sprintf(pexec_prog_name,"%s",path);
	remove_mask(pexec_prog_name);
	pexec_error_code = chdir(pexec_prog_name);
	if(pexec_error_code!=0)
	{
			sprintf(alert_text,"[3][Path does not exist][OK]");
			form_alert(1,alert_text);
	}

	strcat(pexec_prog_name,name_buf);



	/* ask for args */
	rsrc_gaddr(R_TREE,FORM_Ask_For_Arg,&obj);

	set_tedinfo(obj,Ask_Prog_Name,name_buf);
	set_tedinfo(obj,Ask_Args1,"");
	set_tedinfo(obj,Ask_Args2,"");
	set_tedinfo(obj,Ask_Args3,"");

					
	alert_error=handle_dialog(obj,0,0);

	if(alert_error==Ask_Args_CANCEL)
		return 2;

	get_tedinfo(obj,Ask_Args1,name_buf);
	sprintf(temp_args,"%s",name_buf);
	get_tedinfo(obj,Ask_Args2,name_buf);
	strcat(temp_args,name_buf);
	get_tedinfo(obj,Ask_Args3,name_buf);
	strcat(temp_args,name_buf);
		
	parse_args_into(temp_args,tail,127);

	/* normal tos execution */

		wind_get(main_window,WF_CXYWH,	&temp_grect.g_x,
													&temp_grect.g_y,
													&temp_grect.g_w,
													&temp_grect.g_h);

		wind_close(main_window);
		wind_delete(main_window);
		rsrc_gaddr( R_TREE,Normal_Tos,&obj);
		menu_bar(obj,0);
		/*v_clrwk(handle);*/
		v_clsvwk(handle);



/*		sprintf(pexec_prog_name,"%s",virtual_file_system[item].path);
*/
		pexec_error_code = Pexec(0,pexec_prog_name,tail,NULL);

/*					sprintf(alert_text,"[3][pexec return %ld][OK]",pexec_error_code);
					form_alert(1,alert_text);
					sprintf(alert_text,"[3][for %s][OK]",pexec_prog_name);
					form_alert(1,alert_text);
*/
		/* now back to this prog so tidy the screen etc */
		open_workstation();
		/*v_clrwk(handle);*/

		rsrc_gaddr( R_TREE,Normal_Tos,&obj);
		menu_bar(obj,1);

		/* recreate the window */
		main_window_attribs=NAME|CLOSE|FULL|MOVE|SIZE|UPARROW|DNARROW|VSLIDE;

		/* set the window's full state */
		/* by getting the desktop size */
		wind_get(DESK,WF_WXYWH,	&temp_grect.g_x,
										&temp_grect.g_y,
										&temp_grect.g_w,
										&temp_grect.g_h);

		/* set the default full position */
		main_window = wind_create(	main_window_attribs,
											temp_grect.g_x,
											temp_grect.g_y,
											temp_grect.g_w,
											temp_grect.g_h);

		graf_mouse(ARROW,NULL);

		if(main_window<0)
		{
			form_alert(1,"[3][GEM cannot allocate |any more windows.|The program must abort.][ OK ]");
			v_clsvwk(handle);
			appl_exit();
			exit(-1);
		}

		ret = wind_set(main_window,WF_NAME,ADDR("ROOT"));
				wind_get(DESK,WF_WXYWH,	&temp_grect.g_x,
												&temp_grect.g_y,
												&temp_grect.g_w,
												&temp_grect.g_h);
		if(DEFAULT_COLUMNS_ON_SCREEN*cell_width<temp_grect.g_w)
			temp_grect.g_w=DEFAULT_COLUMNS_ON_SCREEN*cell_width;

		if(DEFAULT_LINES_ON_SCREEN*cell_height<temp_grect.g_h)
			temp_grect.g_h=DEFAULT_LINES_ON_SCREEN*cell_height;

		ret = wind_open(main_window,temp_grect.g_x,temp_grect.g_y,temp_grect.g_w,temp_grect.g_h);
/*		ret = wind_open(main_window,temp_grect.g_x,temp_grect.g_y,DEFAULT_COLUMNS_ON_SCREEN*cell_width,DEFAULT_LINES_ON_SCREEN*cell_height);
*/		display_virtual_file_names(main_window,NULL);
		set_bar_size(main_window);
		set_bar_pos(main_window);

	return 0;
}

void handle_double_click( int item)
{

	/*int loop;*/
	int old_item;

	old_item=item;

			/* we got ourselves a valid double click */
/*			for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
			{
				if(virtual_file_system[loop].unique_no!=0 && virtual_file_system[loop].parent==current_dir_level)
				{
					if(item==1)
					{
						/* found it */
						item=loop;
						loop=MAX_VIRTUAL_FILES;
					}
					else
						item--;
				}
			}
*/
			item=unique_from_item(item);

			if(virtual_file_system[item].path!=NULL)
			{
				save_vfs_on(save_vfs_on_exec);
				old_item=unique_from_item(old_item);

				execute_program(old_item);
			}
			else
			{
				/* a dir */
				current_dir_level=virtual_file_system[item].unique_no;
				ret2 = wind_set(main_window,WF_NAME,ADDR(virtual_file_system[item].display_text));

				number_in_this_dir=how_many_in_this_dir(current_dir_level);
				if(number_in_this_dir==0)
					first_on_screen=0;
				else
					first_on_screen=1;
		
				highlight=0;			
				display_virtual_file_names(main_window,NULL);
			}
				  
}

void select_deselect(int item)
{
	GRECT ra;
	short tattributes[10];
	int item_in_window;
	
		if(highlight==item)
		{

			highlight=0;

			/* get window width */
			wind_get(main_window,WF_WXYWH,&ra.g_x,&ra.g_y,&ra.g_w,&ra.g_h);
			
			/* get y of item */
			item_in_window=item-first_on_screen;

			/* get the text attributes */
			vqt_attributes(handle,tattributes);

			/* cell height is in tattributes[9] */
			ra.g_y=ra.g_y+(tattributes[9]*item_in_window);

			if(ra.g_h-ra.g_y<tattributes[9])
				ra.g_h=ra.g_h-ra.g_y;
			else
				ra.g_h=tattributes[9];
	
/*			tattributes[0]=ra.g_x;
			tattributes[1]=ra.g_y;
			tattributes[2]=ra.g_x+100;
			tattributes[3]=ra.g_y;
			tattributes[4]=ra.g_x+100;
			tattributes[5]=ra.g_y+ra.g_h;
			tattributes[6]=ra.g_x;
			tattributes[7]=ra.g_y+ra.g_h;

			sprintf(alert_text,"[2][item %d,| x %d, y %d,| h %d, w %d][OK]",item_in_window,ra.g_x,ra.g_y,ra.g_h,ra.g_w);
			v_pline(handle,4,tattributes);
			form_alert(1,alert_text); 
*/
			display_virtual_file_names(main_window,&ra);

		}
		else
		{
			highlight=item;
			display_virtual_file_names(main_window,NULL);
		}

}

int how_many_in_this_dir(long dir_number)
{
	int loop;
	int count;

	loop=0;
	count =0;

	while(loop<current_virtual_files)
	{
		if(virtual_file_system[loop].unique_no!=0)
		{

			if(virtual_file_system[loop].parent==dir_number)
			{
				count++;
			}
		}

		loop++;
	}

	return count;
}

void free_up_the_virtual_files( void )
{
	int loop;

	loop=0;

	if(virtual_file_system!=NULL)
	{
	while(loop<MAX_VIRTUAL_FILES)
	{
		if(virtual_file_system[loop].unique_no!=0)
		{
			if(virtual_file_system[loop].path!=NULL)
				free(virtual_file_system[loop].path);
			if(virtual_file_system[loop].working_dir!=NULL)
				free(virtual_file_system[loop].working_dir);
			if(virtual_file_system[loop].args!=NULL)
				free(virtual_file_system[loop].args);
			if(virtual_file_system[loop].display_text!=NULL)
				free(virtual_file_system[loop].display_text);
		}

		loop++;
	}

	free(virtual_file_system);
	}
	virtual_file_system=NULL;
}

int max_in_window(int window_handle)
{
	short x,y,w,h,tattributes[10];

	/* get the text attributes */
	vqt_attributes(handle,tattributes);

	/* cell height is in tattributes[9] */

	/*get the window work area */
	wind_get(window_handle,WF_WXYWH,&x,&y,&w,&h);

	return (h/tattributes[9]);
}

void set_bar_pos(int window_handle)
{
	int bar_pos;
	
	bar_pos = (int)((float)((float)number_in_this_dir/1000.0)*(float)first_on_screen);

/*
	long bar_pos;
	int number_in_window;

	number_in_window=max_in_window(window_handle);

	bar_pos = 1000*first_on_screen/(number_in_this_dir-number_in_window);
*/
	wind_set(window_handle,WF_VSLIDE,bar_pos);

}

void set_first_item(int window_handle)
{
	short bar_pos;
	short dummy;
	int number_in_window;

	number_in_window=max_in_window(window_handle);

	wind_get(window_handle,WF_VSLIDE,&bar_pos,&dummy,&dummy,&dummy);

	if(bar_pos==0 && number_in_this_dir!=0)
		first_on_screen=1;
	else
	{
		if(number_in_window<number_in_this_dir)
			first_on_screen =(int)((float)(((float)number_in_this_dir-(float)(number_in_window/*-1*/))/1000.0)*(float)bar_pos);

/*			first_on_screen =bar_pos * (number_in_this_dir-number_in_window) /1000;
*/		else
			first_on_screen =(int)((float)((float)number_in_this_dir/1000.0)*(float)bar_pos);
	}
}

void set_bar_size(int window_handle)
{
	int bar_size; 
	int number_in_window;

	number_in_window=max_in_window(window_handle);


	if(number_in_this_dir<=number_in_window)
	{
		bar_size=1000;
	}
	else
		bar_size=(int)((float)((float)number_in_window/(float)number_in_this_dir)*1000.0);

/*		bar_size=(1000 * number_in_window) / number_in_this_dir;
*/
/*	sprintf(alert_text,"[1][in win %d,| in dir %d,| bar_size %d][OK]",number_in_window,number_in_this_dir,bar_size);
	form_alert(1,alert_text);
*/
	wind_set(window_handle,WF_VSLSIZE,bar_size);	
}

void redraw_window( int window_handle, GRECT *dirty)
{
	GRECT r;

	wind_update(BEG_UPDATE);

	wind_get(window_handle, WF_FIRSTXYWH, &r.g_x, &r.g_y,&r.g_w,&r.g_h);
	while(r.g_w && r.g_h)
	{
		if(rc_intersect(dirty, &r))
		{
			display_virtual_file_names(window_handle,&r);
		}
		wind_get(window_handle, WF_NEXTXYWH, &r.g_x, &r.g_y,&r.g_w,&r.g_h);
	}

	wind_update(END_UPDATE);
}


void display_virtual_file_names(int window_handle,GRECT *clip_area)
{
	
	int loop;
	int count;
	int printed;
	int printable;
	GRECT c;
	short tattributes[10];
	int y_offset;
	int x_offset;
	char ident_char[2];

	wclip(window_handle,clip_area);
	clearw(window_handle);

	loop=0;
	count=1;
	printable = max_in_window(window_handle) + 1; /* to be on the safe side */
	printed=0;

	wind_get(window_handle,WF_WXYWH,&c.g_x,&c.g_y,&c.g_w,&c.g_h);

	/* get the text attributes */
	vqt_attributes(handle,tattributes);

	/* cell height is in tattributes[9] */
	y_offset=c.g_y+tattributes[9];
	x_offset=10;

	graf_mouse(M_OFF,NULL);

	while(loop<current_virtual_files && count<=number_in_this_dir && printed<=printable)
	{

		if(current_dir_level==0)
			ret2 = wind_set(main_window,WF_NAME,ADDR("ROOT"));
		else
			if(virtual_file_system[loop].unique_no==current_dir_level)
				ret2 = wind_set(window_handle,WF_NAME,ADDR(virtual_file_system[loop].display_text));


		if(virtual_file_system[loop].unique_no!=0)
		{
			/* possibly print? */
			if(virtual_file_system[loop].parent==current_dir_level)
			{
				/*	yes	*/

				/* is it on screen? */
				if(first_on_screen<=count)
				{
					/* yes */

					/* is it highlit? */
					if( count==highlight)
					{
						/* yes */
						vswr_mode(handle,MD_ERASE);
					}
					else
					{
						/* no */
						vswr_mode(handle,MD_REPLACE);
					}

					if(virtual_file_system[loop].path==NULL)
						ident_char[0]='\\';		/* dir symbol */
					else
						ident_char[0]=' ';

					ident_char[1]='\0';

					v_gtext(handle,c.g_x+x_offset,((tattributes[9]*printed)+y_offset),ident_char);
					v_gtext(handle,c.g_x+x_offset+tattributes[8],((tattributes[9]*printed)+y_offset),virtual_file_system[loop].display_text);

					printed++;
				}	
				count++;
			}
		}

		loop++;
	}

	vswr_mode(handle,MD_REPLACE);
	graf_mouse(M_ON,NULL);

}

void edit_item()
{
	int loop;
	int item;
/*	long dl;*/

	item =highlight;
	
	if(item==0)
		form_alert(1,"[2][No item is highlighted][OK]");
	else
	{
 
			/* we got ourselves a valid double click */
			item=unique_from_item(item);

		if(virtual_file_system[item].path==NULL)	/* a directory */
		{
			loop=create_directory_name_for(virtual_file_system[item].display_text);
			if(loop==1)
				vfs_has_changed=1;
		}
		else
		{

			if(virtual_file_system[item].execute_by=='D')
			{

/*sprintf(alert_text,"[1][prog unique = %ld][ok]",virtual_file_system[item].program_unique_no);
form_alert(1,alert_text);
*/
				loop=create_document_for(		virtual_file_system[item].display_text,
														&virtual_file_system[item].path,
														&virtual_file_system[item].args,
														&virtual_file_system[item].working_dir,
														&virtual_file_system[item].execute_by,
														&virtual_file_system[item].date_created,
														&virtual_file_system[item].current_version,
														&virtual_file_system[item].date_last_edit,
														&virtual_file_system[item].program_unique_no

												);
				if(loop==1)
					vfs_has_changed=1;
			}
			else
			{
				/* a program */

				loop=create_program_name_for(virtual_file_system[item].display_text,&virtual_file_system[item].path,&virtual_file_system[item].args_type,&virtual_file_system[item].args,&virtual_file_system[item].working_dir);
				if(loop==1)
					vfs_has_changed=1;
			}
		}

	}

}

void copy_item()
{
	/*int loop;*/
	int item;

	item =highlight;
	
	if(item==0)
		form_alert(1,"[2][No item is highlighted][OK]");
	else
	{
 
			/* we got ourselves a valid double click */
/*			for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
			{
				if(virtual_file_system[loop].unique_no!=0 && virtual_file_system[loop].parent==current_dir_level)
				{
					if(item==1)
					{
						/* found it */
						item=loop;
						loop=MAX_VIRTUAL_FILES;
					}
					else
						item--;
				}
			}
*/
		item=unique_from_item(item);

		/* tag the stuff onto the temp_virtual_file */
		if(temp_virtual_file_active==1)
		{
			/* get rid of old stuff */
				if(temp_virtual_file.path!=NULL)
					free(temp_virtual_file.path);
				temp_virtual_file.path=NULL;

				if(temp_virtual_file.args!=NULL)
					free(temp_virtual_file.args);
				temp_virtual_file.args=NULL;
				temp_virtual_file.args_type=0;

				if(temp_virtual_file.working_dir!=NULL)
					free(temp_virtual_file.working_dir);
				temp_virtual_file.working_dir=NULL;

				if(temp_virtual_file.display_text!=NULL)
					free(temp_virtual_file.display_text);
				temp_virtual_file.display_text=NULL;

				temp_virtual_file.unique_no=0;
				temp_virtual_file.parent=0;

				temp_virtual_file_active=0;
		}

		/* now copy item details to temp */

				if(virtual_file_system[item].path!=NULL)
				{
					temp_virtual_file.path=(char *)malloc(FMSIZE+FNSIZE);
					if(temp_virtual_file.path!=NULL)
						strcpy(temp_virtual_file.path,virtual_file_system[item].path);
					else
						temp_virtual_file.path=NULL;
				}
				else
					temp_virtual_file.path=NULL;

				if(virtual_file_system[item].args!=NULL)
				{
					temp_virtual_file.args=(char *)malloc(91);
					if(temp_virtual_file.args!=NULL)
						strcpy(temp_virtual_file.args,virtual_file_system[item].args);
					else
						temp_virtual_file.args=NULL;
				}
				else
					temp_virtual_file.args=NULL;

				temp_virtual_file.args_type=virtual_file_system[item].args_type;

				if(virtual_file_system[item].working_dir!=NULL)
				{
					temp_virtual_file.working_dir=(char *)malloc(FMSIZE+FNSIZE);
					if(temp_virtual_file.working_dir!=NULL)
						strcpy(temp_virtual_file.working_dir,virtual_file_system[item].working_dir);
					else
						temp_virtual_file.working_dir=NULL;
				}
				else
					temp_virtual_file.working_dir=NULL;

				if(virtual_file_system[item].display_text!=NULL)
				{
					temp_virtual_file.display_text=(char *)malloc(40);
					if(temp_virtual_file.display_text!=NULL)
						strcpy(temp_virtual_file.display_text,virtual_file_system[item].display_text);
					else
						temp_virtual_file.display_text=NULL;
				}
				else
					temp_virtual_file.display_text=NULL;

				temp_virtual_file.unique_no=0;
				temp_virtual_file.parent=0;
				temp_virtual_file.execute_by=virtual_file_system[item].execute_by;
				temp_virtual_file_active=1;
				temp_virtual_file.date_created=virtual_file_system[item].date_created;
				temp_virtual_file.current_version=virtual_file_system[item].current_version;
				temp_virtual_file.date_last_edit=virtual_file_system[item].date_last_edit;
				temp_virtual_file.program_unique_no=virtual_file_system[item].program_unique_no;
	}

}

void cut_item()
{
	/*int loop;*/
	int item;

	item =highlight;
	
	if(item==0)
		form_alert(1,"[2][No item is highlighted][OK]");
	else
	{
		copy_item();
		delete_highlight();
	}
}

void paste_item()
{
	/*int loop;*/
	int item;
	char *path, *working_dir, *display_text, *args;

	/*item =highlight;*/
	
	if(temp_virtual_file_active==0)
		form_alert(1,"[2][No item in cut&paste buffer][OK]");
	else
	{
/*sprintf(alert_text,"[1][adding %d.%d|%s|%s][ok]",current_dir_level,
										unique_number_count,
										temp_virtual_file.path,				/* exec path or NULL if directory */
										temp_virtual_file.display_text);
form_alert(1,alert_text);

sprintf(alert_text,"[1][adding |%d %s][ok]",
										temp_virtual_file.args_type,
										temp_virtual_file.args
);		/* 'M' for Multitask, 'H' for Hog, 'E' for exit */
form_alert(1,alert_text);

sprintf(alert_text,"[1][adding %s|%c][ok]",
										temp_virtual_file.working_dir,
										temp_virtual_file.execute_by);		/* 'M' for Multitask, 'H' for Hog, 'E' for exit */
form_alert(1,alert_text);
*/

				if(temp_virtual_file.path!=NULL)
				{
					path=(char *)malloc(FMSIZE+FNSIZE);
					if(path!=NULL)
						strcpy(path,temp_virtual_file.path);
					else
						path=NULL;
				}
				else
					path=NULL;

				if(temp_virtual_file.args!=NULL)
				{
					args=(char *)malloc(91);
					if(args!=NULL)
						strcpy(args,temp_virtual_file.args);
					else
						args=NULL;
				}
				else
					args=NULL;

				if(temp_virtual_file.working_dir!=NULL)
				{
					working_dir=(char *)malloc(FMSIZE+FNSIZE);
					if(temp_virtual_file.working_dir!=NULL)
						strcpy(working_dir,temp_virtual_file.working_dir);
					else
						working_dir=NULL;
				}
				else
					working_dir=NULL;

				if(temp_virtual_file.display_text!=NULL)
				{
					display_text=(char *)malloc(40);
					if(display_text!=NULL)
						strcpy(display_text,temp_virtual_file.display_text);
					else
						display_text=NULL;
				}
				else
					display_text=NULL;




		item=add_item_given_details(	current_dir_level,			/* 0 if root level */
										unique_number_count,
										/*temp_virtual_file.*/path,				/* exec path or NULL if directory */
										/*temp_virtual_file.*/display_text,
										temp_virtual_file.args_type,
										/*temp_virtual_file.*/args,
										/*temp_virtual_file.*/working_dir,
										temp_virtual_file.execute_by,		/* 'M' for Multitask, 'H' for Hog, 'E' for exit */
										temp_virtual_file.date_created,
										temp_virtual_file.current_version,
										temp_virtual_file.date_last_edit,
										temp_virtual_file.program_unique_no);		



		unique_number_count++;
		current_virtual_files++;
		number_in_this_dir++;
		if(number_in_this_dir==1)
			first_on_screen=1;
	
		sort_virtual_files();
		vfs_has_changed=1;

/*sprintf(alert_text,"[1][return add is %d][ok]",item);
form_alert(1,alert_text);
*/
	}
} 

void add_item()
{

	int loop;
	int free_slot;
	int dir_prog;
/*	short but;*/
	char *cp;
	char dir_name[35];
	char *prog_name;
	int args_type;
	char *args_string;
	char *working_dir;
/*	long dum_long;*/
	long l_ver,l_last_edit,l_package,l_create;
	char ex_by;

	/* any more slots? */

	if(current_virtual_files>=MAX_VIRTUAL_FILES)
		/* eeek no more slots, try and resize */
		virtual_file_system=expand_virtual_file_system(MAX_VIRTUAL_FILES);

	/* check if we have space */
	if(current_virtual_files<MAX_VIRTUAL_FILES)
	{
		/* yes */

		free_slot=0;

		/* find a free slot */
		for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
			if(virtual_file_system[loop].unique_no==0)
			{
				free_slot=loop;
				loop=MAX_VIRTUAL_FILES;
			}

		/* adding dir or prog? */
/*		dir_prog=form_alert(1,"[2][  Do you want to create a:   ][Directory|Program|Cancel]");
*/		dir_prog=create_what();
	
		if(dir_prog!=4)
		{
			/* show the file selector */
			if(dir_prog==1)
			{
/*				ret2=fsel_exinput(path,filename,&but,"INPUT DIRECTORY NAME");
*/
				dir_name[0]='\0';

				ret2=create_directory_name_for(dir_name);
				if(ret2!=0)
				{
					cp=stpblk(dir_name);
					if(*cp!='\0')
					{
						/* add it */
						virtual_file_system[free_slot].path=NULL;
						virtual_file_system[free_slot].working_dir=NULL;
						virtual_file_system[free_slot].args_type=0;
						virtual_file_system[free_slot].args=NULL;
						virtual_file_system[free_slot].display_text=(char *)malloc(35);
						if(virtual_file_system[free_slot].display_text==NULL)
						{
							form_alert(1,"[3][SORRY - Memory Error |no more space.][ OK ]");
							goto skip_here;
						}
	
						sprintf(virtual_file_system[free_slot].display_text,"%s",dir_name);
						virtual_file_system[free_slot].parent=current_dir_level;
						virtual_file_system[free_slot].unique_no=unique_number_count;
	
						if(first_on_screen==0)
							first_on_screen=1;
						else
						{
							/* might need to scroll to fit the item on screen */
						}
						vfs_has_changed=1;
						unique_number_count++;
						current_virtual_files++;
						number_in_this_dir++;
						sort_virtual_files();
					}
					else
						form_alert(1,"[3][SORRY - Blank Names not allowed][ OK ]");

				}

			}
			else
			{
				if(dir_prog==2)
				{
					dir_name[0]='\0';
					prog_name=NULL;
					args_type=0;
					args_string=NULL;
					working_dir=NULL;
					ret2=create_program_name_for(dir_name,&prog_name,&args_type,&args_string,&working_dir);
					if(ret2!=0)
					{
							virtual_file_system[free_slot].path=prog_name;
							virtual_file_system[free_slot].working_dir=working_dir;
							virtual_file_system[free_slot].args_type=args_type;
							virtual_file_system[free_slot].args=args_string;
							if(virtual_file_system[free_slot].path==NULL)
							{
								form_alert(1,"[3][SORRY - Memory Error |no more space.][ OK ]");
								goto skip_here;
							}
			
							virtual_file_system[free_slot].display_text=(char *)malloc(40);
							if(virtual_file_system[free_slot].display_text==NULL)
							{
								form_alert(1,"[3][SORRY - Memory Error |no more space.][ OK ]");
								goto skip_here;
							}
							else
							{
								sprintf(virtual_file_system[free_slot].display_text,"%s",dir_name);
							}	
			
							virtual_file_system[free_slot].parent=current_dir_level;
							virtual_file_system[free_slot].unique_no=unique_number_count;
							virtual_file_system[free_slot].execute_by=' ';
		
							if(first_on_screen==0)
								first_on_screen=1;
							else
							{
								/* might need to scroll to fit the item on screen */
							}
							vfs_has_changed=1;
							unique_number_count++;
							current_virtual_files++;
							number_in_this_dir++;
							sort_virtual_files();
					}
				}
				else
				{
					if(dir_prog==3)
					{
						dir_name[0]='\0';
						prog_name=NULL;
						args_string=NULL;
						working_dir=NULL;
						l_ver=1;
						l_last_edit=0;
						l_package=-1;
						l_create=time(NULL); /*Gettime();*/

					 	ret2=create_document_for(dir_name,&prog_name,&args_string,&working_dir,&ex_by,&l_create,&l_ver,&l_last_edit,&l_package);
						if(ret2!=0)
						{
							virtual_file_system[free_slot].path=prog_name;
							virtual_file_system[free_slot].working_dir=working_dir;
							virtual_file_system[free_slot].args=args_string;
							virtual_file_system[free_slot].args_type=0;
							if(virtual_file_system[free_slot].path==NULL)
							{
								form_alert(1,"[3][SORRY - Memory Error |no more space.][ OK ]");
								goto skip_here;
							}
			
							virtual_file_system[free_slot].display_text=(char *)malloc(40);
							if(virtual_file_system[free_slot].display_text==NULL)
							{
								form_alert(1,"[3][SORRY - Memory Error |no more space.][ OK ]");
								goto skip_here;
							}
							else
							{
								sprintf(virtual_file_system[free_slot].display_text,"%s",dir_name);
							}	
			
							virtual_file_system[free_slot].parent=current_dir_level;
							virtual_file_system[free_slot].unique_no=unique_number_count;
							virtual_file_system[free_slot].execute_by='D';
							virtual_file_system[free_slot].date_created=l_create;
							virtual_file_system[free_slot].current_version=1;
							virtual_file_system[free_slot].date_last_edit=0;
							virtual_file_system[free_slot].program_unique_no=l_package;
		
							if(first_on_screen==0)
								first_on_screen=1;
							else
							{
								/* might need to scroll to fit the item on screen */
							}
							vfs_has_changed=1;
							unique_number_count++;
							current_virtual_files++;
							number_in_this_dir++;
							sort_virtual_files();
						}
					}
				}
			}	
		}

	}
	else
	{
		/* NO */
		form_alert(1,"[3][SORRY - No more memory for|virtual file system.][ OK ]");	
	}

skip_here:;

}

int add_item_given_details(	long parent,			/* 0 if root level */
										long unique_no,
										char *path,				/* exec path or NULL if directory */
										char *display_text,
										int args_type,
										char *args,
										char *working_dir,
										char execute_by,
										long date_created,
										long current_version,
										long date_last_edit,
										long program_unique_no)		
{

	int loop;
	int free_slot;

	/* any more slots? */

	if(current_virtual_files>=MAX_VIRTUAL_FILES)
		/* eeek no more slots, try and resize */
		virtual_file_system=expand_virtual_file_system(MAX_VIRTUAL_FILES);

	/* check if we have space */
	if(current_virtual_files<MAX_VIRTUAL_FILES)
	{
		/* yes */

		free_slot=0;

		/* find a free slot */
		for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
			if(virtual_file_system[loop].unique_no==0)
			{
				free_slot=loop;
				loop=MAX_VIRTUAL_FILES;
			}

/*sprintf(alert_text,"[1][adding at slot %d, unique is %d][ok]",free_slot,unique_no);
form_alert(1,alert_text);
*/
		virtual_file_system[free_slot].path=path;
		virtual_file_system[free_slot].display_text=display_text;
		virtual_file_system[free_slot].parent=parent;
		virtual_file_system[free_slot].unique_no=unique_no;
		virtual_file_system[free_slot].args_type=args_type;
		virtual_file_system[free_slot].args=args;
		virtual_file_system[free_slot].working_dir=working_dir;
		virtual_file_system[free_slot].execute_by=execute_by;		/* 'M' for Multitask, 'H' for Hog, 'E' for exit */
		virtual_file_system[free_slot].date_created=date_created;
		virtual_file_system[free_slot].current_version=current_version;		
		virtual_file_system[free_slot].date_last_edit=date_last_edit;		
		virtual_file_system[free_slot].program_unique_no=program_unique_no;		

		return 0;		
	}
	else
	{
		/* NO */

		return 1;
	}
}


void rationalise_array()
{
	int loop;
	int count;
	int found;
	int loop2;

	count =0;

	/* go through them all and count the ones that are used */
	for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
		if(virtual_file_system[loop].unique_no!=0)
			count++;


	/* go through all the nums from 1 to number of items */
	/* and make sure that only these numbers are used */
	/* any that are unused, those items above them get */
	/* renumbered to match */ 
	for(loop=1;loop<=count;loop++)
	{
		/* find loop.unique_no */
		found = 0;

		for(loop2=0;loop2<MAX_VIRTUAL_FILES;loop2++)
			if(virtual_file_system[loop2].unique_no==loop)
				found=1;

		if(found==0)
			/* need to shunt up the rest to match this */
			rationalise_unique_no(loop);


	}

}

void rationalise_unique_no(int need_unique)
{
	int closest_greater_no,closest_greater_index;
	int loop;

	closest_greater_no = 0;

	for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
	{
		if(virtual_file_system[loop].unique_no > need_unique && closest_greater_no ==0)
		{
			closest_greater_no = virtual_file_system[loop].unique_no;
			closest_greater_index=loop;
		}
		else
			if(virtual_file_system[loop].unique_no > need_unique && virtual_file_system[loop].unique_no<closest_greater_no)
			{
				closest_greater_no = virtual_file_system[loop].unique_no;
				closest_greater_index=loop;
			}
	}

	renumber_unique(closest_greater_no,need_unique,closest_greater_index);
}

void renumber_unique(int old_unique, int new_unique,int old_unique_index)
{
	int loop;

	/* if it has kids then tell the kid what the parent is now called */
	for(loop = 0; loop < MAX_VIRTUAL_FILES; loop++)
	{
		if(virtual_file_system[loop].unique_no!=0 && virtual_file_system[loop].parent==old_unique)
			virtual_file_system[loop].parent=new_unique;

		if(virtual_file_system[loop].unique_no!=0 && virtual_file_system[loop].execute_by=='D' && virtual_file_system[loop].program_unique_no==old_unique)
			virtual_file_system[loop].program_unique_no=new_unique;
	}

	if(virtual_file_system[old_unique_index].unique_no==current_dir_level);
		current_dir_level=new_unique;

	virtual_file_system[old_unique_index].unique_no=new_unique;

}

int dir_has_children(int unique_no)
{
	int loop;

	for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
	{
		if(virtual_file_system[loop].unique_no!=0)
			if(virtual_file_system[loop].parent==unique_no)
				return 1;
	}
	
	return 0;
}

void move_children_up_a_level(int old_parent)
{
	int loop;
	int new_parent;

	loop=array_index_no(old_parent);

	new_parent=virtual_file_system[loop].parent;

	for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
	{
		if(virtual_file_system[loop].unique_no!=0)
			if(virtual_file_system[loop].parent==old_parent)
				virtual_file_system[loop].parent=new_parent;
	}
	
}

void scrub_array_entry(int highlights_index)
{
				if(virtual_file_system[highlights_index].path!=NULL)
					free(virtual_file_system[highlights_index].path);
				virtual_file_system[highlights_index].path=NULL;

				if(virtual_file_system[highlights_index].args!=NULL)
					free(virtual_file_system[highlights_index].args);
				virtual_file_system[highlights_index].args=NULL;
				virtual_file_system[highlights_index].args_type=0;

				if(virtual_file_system[highlights_index].working_dir!=NULL)
					free(virtual_file_system[highlights_index].working_dir);
				virtual_file_system[highlights_index].working_dir=NULL;

				if(virtual_file_system[highlights_index].display_text!=NULL)
					free(virtual_file_system[highlights_index].display_text);
				virtual_file_system[highlights_index].display_text=NULL;

				virtual_file_system[highlights_index].unique_no=0;
				virtual_file_system[highlights_index].parent=0;
}

int array_index_no(int unique_no)
{
	int loop;

	for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
	{
		if(virtual_file_system[loop].unique_no==unique_no)
			return loop;
	}
	
	return -1;
}

int first_child_of(int parent)
{
	int loop;

	for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
	{
		if(virtual_file_system[loop].parent==parent)
			return virtual_file_system[loop].unique_no;
	}
	
	return -1;
}

void scrub_dir(int original_parent)
{
	int delete_this;
	int delete_parent;
	int stop_parent;
	int index;

	index=array_index_no(original_parent);
	stop_parent=virtual_file_system[index].parent;
	delete_parent=original_parent;

	while(delete_parent!=stop_parent)
	{
		if(dir_has_children(delete_parent))
		{
			delete_parent=first_child_of(delete_parent);
		}
		else
		{
			delete_this=array_index_no(delete_parent);
			delete_parent=virtual_file_system[array_index_no(delete_parent)].parent;
			scrub_array_entry(delete_this);
		}
	}

}

void delete_highlight()
{
	int loop;
	int count;
	int highlights_unique_no;
	int highlights_index;
	int del_add;

	if(highlight!=0)
	{
		/* find highlight's unique_no */

		count =0;
		for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
		{
			if(virtual_file_system[loop].unique_no!=0)
				if(virtual_file_system[loop].parent==current_dir_level)
				{
					count++;
					if(count == highlight)
					{
						highlights_unique_no=virtual_file_system[loop].unique_no;
						highlights_index=loop;
						loop=MAX_VIRTUAL_FILES;
					}
				}
		}			

	/* is it a dir? */
	
		if(virtual_file_system[highlights_index].path==NULL)
		{
			/* yes */
			
			/* does it have children */

			if(dir_has_children(highlights_unique_no))
			{
				/* yes */
				del_add=form_alert(1,"[3][This directory has items in it|do you want to delete them|or add them to the parent|directory][DELETE|ADD|CANCEL]");
				if(del_add!=3)
				{
					if(del_add==2)	/* add */
					{
						move_children_up_a_level(highlights_unique_no);
						scrub_array_entry(highlights_index);
					}
					else
					{
						/* delete the lot of them */
						scrub_dir(highlights_unique_no);
					}
					vfs_has_changed=1;
				}

			}
			else
			{
				/* no */
				/* OK scrub it */
				scrub_array_entry(highlights_index);
				vfs_has_changed=1;
			}
		}
		else
		{
				/* no */

				/* is the prog referenced? */
				remove_symbolic_links_for(highlights_index);
				/* OK scrub it */
				scrub_array_entry(highlights_index);
		}
	
		sort_virtual_files();


	number_in_this_dir=how_many_in_this_dir(current_dir_level);
	}

}	


void sort_virtual_files()
{
	int loop;
	int progs_start_at;
	int swap_made;
	virtual_file temp_file;


	/* sort so that stuff come first followed by blank */
/*	form_alert(1,"[3][Sorting blanks][ OK ]");
*/	swap_made=1;
	while(swap_made==1)
	{
		loop=0;
		swap_made=0;

		while(loop<MAX_VIRTUAL_FILES && loop+1<MAX_VIRTUAL_FILES)
		{
			if(virtual_file_system[loop].unique_no==0)
			{
				if(virtual_file_system[loop].unique_no!=virtual_file_system[loop+1].unique_no)
				{
					memcpy(&temp_file,&virtual_file_system[loop],sizeof(virtual_file));
					memcpy(&virtual_file_system[loop],&virtual_file_system[loop+1],sizeof(virtual_file));
					memcpy(&virtual_file_system[loop+1],&temp_file,sizeof(virtual_file));					
					swap_made=1;
				}
			}
			loop++;
		}			
	}

	/* sort so that dirs come first followed by progs */
/*	form_alert(1,"[3][Sorting dirs][ OK ]");
*/	swap_made=1;
	while(swap_made==1)
	{
		loop=0;
		swap_made=0;

		while(loop<MAX_VIRTUAL_FILES && loop+1<MAX_VIRTUAL_FILES &&
				virtual_file_system[loop].unique_no!=0 &&
				virtual_file_system[loop+1].unique_no!=0)
		{
			if(virtual_file_system[loop].path!=NULL)
			{
				if(virtual_file_system[loop+1].path==NULL)
				{
					memcpy(&temp_file,&virtual_file_system[loop],sizeof(virtual_file));
					memcpy(&virtual_file_system[loop],&virtual_file_system[loop+1],sizeof(virtual_file));
					memcpy(&virtual_file_system[loop+1],&temp_file,sizeof(virtual_file));					
					swap_made=1;
				}
			}
			loop++;
		}			
	}

	/* sort dirs into alphabetic order */
/*	form_alert(1,"[3][Sorting dirs alpha][ OK ]");
*/
	swap_made=1;
	while(swap_made==1)
	{
		loop=0;
		swap_made=0;

		while(loop<MAX_VIRTUAL_FILES && loop+1<MAX_VIRTUAL_FILES &&
				virtual_file_system[loop].path==NULL &&
				virtual_file_system[loop+1].path==NULL &&
				virtual_file_system[loop].unique_no!=0 &&
				virtual_file_system[loop+1].unique_no!=0)
		{
			if(stricmp(virtual_file_system[loop].display_text,virtual_file_system[loop+1].display_text)>0)
			{
					memcpy(&temp_file,&virtual_file_system[loop],sizeof(virtual_file));
					memcpy(&virtual_file_system[loop],&virtual_file_system[loop+1],sizeof(virtual_file));
					memcpy(&virtual_file_system[loop+1],&temp_file,sizeof(virtual_file));					
					swap_made=1;
			}
			loop++;
		}			
	}

	/* sort progs into alphabetic order */

/*	form_alert(1,"[3][Sorting progs][ OK ]");
*/	for(loop=0;loop<MAX_VIRTUAL_FILES;loop++)
		if(virtual_file_system[loop].path!=NULL)
		{
			progs_start_at=loop;
			loop=MAX_VIRTUAL_FILES;
		}

	swap_made=1;
	while(swap_made==1)
	{
		loop=progs_start_at;
		swap_made=0;

		while(loop<MAX_VIRTUAL_FILES && loop+1<MAX_VIRTUAL_FILES &&
				virtual_file_system[loop].unique_no!=0 &&
				virtual_file_system[loop+1].unique_no!=0)
		{
			if(stricmp(virtual_file_system[loop].display_text,virtual_file_system[loop+1].display_text)>0)
			{
					memcpy(&temp_file,&virtual_file_system[loop],sizeof(virtual_file));
					memcpy(&virtual_file_system[loop],&virtual_file_system[loop+1],sizeof(virtual_file));
					memcpy(&virtual_file_system[loop+1],&temp_file,sizeof(virtual_file));					
					swap_made=1;
			}
			loop++;
		}			
	}
}

void clearw(int window_handle)
{
		short coord[4];
		GRECT c;

		wind_get(window_handle,WF_WXYWH,&c.g_x,&c.g_y,&c.g_w,&c.g_h);

		coord[0]=c.g_x;
		coord[1]=c.g_y;
		coord[2]=c.g_x+c.g_w-1;
		coord[3]=c.g_y+c.g_h-1;

		
		graf_mouse(M_OFF,NULL);
		vsf_color(handle,0);
		vr_recfl(handle,coord);
		graf_mouse(M_ON,NULL);
}

void wclip(int window_handle,GRECT *clip_area)
{
		short coord[4];

		GRECT c;

		if(clip_area==NULL)
		{
			wind_get(window_handle,WF_WXYWH,&c.g_x,&c.g_y,&c.g_w,&c.g_h);
		}
		else
			memcpy(&c,clip_area,sizeof(GRECT));

			coord[0]=c.g_x;
			coord[1]=c.g_y;
			coord[2]=c.g_x+c.g_w-1;
			coord[3]=c.g_y+c.g_h-1;
		
		vs_clip(handle,1,coord);

}

void fullw(int window_handle)
{
	GRECT c,p,f;

	wind_get(window_handle,WF_CXYWH,&c.g_x,&c.g_y,&c.g_w,&c.g_h);
	wind_get(window_handle,WF_FXYWH,&f.g_x,&f.g_y,&f.g_w,&f.g_h);

	if(rc_equal(&c,&f))
	{
		wind_get(window_handle,WF_PXYWH,&p.g_x,&p.g_y,&p.g_w,&p.g_h);
		if(!rc_equal(&p,&f))
		{
				wind_set(window_handle,WF_CXYWH,p.g_x,p.g_y,p.g_w,p.g_h);
		}
	}
	else
	{
			wind_set(window_handle,WF_CXYWH,f.g_x,f.g_y,f.g_w,f.g_h);
	}
			
}



void remove_mask(char *path_string)
{
	/*find the end of the string and work back until you find \\*/
	char *cp;

	cp=path_string;
	cp=cp+strlen(path_string)-1;
	while(*cp!='\\')
		cp--;
	cp++;
	*cp='\0';

}


int create_directory_name_for(char *dir_name)
	{
	char name_buf[40];
	OBJECT *dlog;
	int result;
	
	rsrc_gaddr(R_TREE,FORM_Create_Dire,&dlog);

	if(dir_name==NULL)
		sprintf(name_buf,"");
	else
		sprintf(name_buf,"%s",dir_name);

	set_tedinfo(dlog,V_Dir_Name,name_buf);

	result=handle_dialog(dlog,0,0);

	if(result==VDir_OK)
	{
		get_tedinfo(dlog,V_Dir_Name,dir_name);
		return 1;
	}
	else
		return 0;
	
}

int create_what(void)
	{
	OBJECT *dlog;
	int result;
	int default_create;

	default_create=1;

	rsrc_gaddr(R_TREE,FORM_Create_What,&dlog);


/*	switch(default_create)
	{
		case 1:
					set_button(dlog,Create_What_Box,Create_DIR);		
					break;
		case 2:
					set_button(dlog,Create_What_Box,Create_PROG);		
					break;
		case 3:
					set_button(dlog,Create_What_Box,Create_DOC);		
					break;
	}
*/

	result=handle_dialog(dlog,0,0);

	if(result==Create_OK)
	{
		result=get_button(dlog,Create_What_Box);

		switch(result)
		{
		case Create_DIR:
					return 1;		
					break;
		case Create_PROG:
					return 2;
					break;
		case Create_DOC:
					return 3;
					break;
		}

		return 4;
	}
	else
		return 4;
	
}

int create_program_name_for(char *prog_name,char **prog_path,int *args_type,char **args_string,char **working_dir)
	{
	char name_buf[40];
	OBJECT *dlog;
	int result;
	int fs_result;
	short but;
	char *cp;
	char temp_filename[FNSIZE+FMSIZE];
	char temp_working_dir[FNSIZE+FMSIZE];
	char arg1[31],arg2[31],arg3[31];
	int args=0;
	int wd;
	
	rsrc_gaddr(R_TREE,FORM_Create_PRog,&dlog);

	args=*args_type;
	arg1[0]='\0';
	arg2[0]='\0';
	arg3[0]='\0';

	if(prog_name==NULL)
	{
		sprintf(name_buf,"");
		sprintf(temp_working_dir,"");
		sprintf(temp_filename,"");
		set_tedinfo(dlog,VProg_Name,name_buf);
		set_tedinfo(dlog,VProg_Path,name_buf);
		set_tedinfo(dlog,Arg_Line1,arg1);
		set_tedinfo(dlog,Arg_Line2,arg2);
		set_tedinfo(dlog,Arg_Line3,arg3);
		set_button(dlog,Argument_Box,Argument_None);		
		set_tedinfo(dlog,Working_Dir_Text,name_buf);
		set_button(dlog,Working_Dir_Box,Working_Program);		
	}
	else
	{
		if(prog_name==NULL)
		{
			sprintf(name_buf,"");
			set_tedinfo(dlog,VProg_Name,name_buf);
		}
		else
		{
			stccpy(name_buf,prog_name,31);
			set_tedinfo(dlog,VProg_Name,name_buf);
		}

		if(*prog_path==NULL)
		{
			sprintf(name_buf,"");
			sprintf(temp_filename,"");
			set_tedinfo(dlog,VProg_Path,name_buf);
		}
		else
		{
/*sprintf(alert_text,"[1][path len %d|%s][ok]",strlen(*prog_path),*prog_path);
form_alert(1,alert_text);
*/

			/*stccpy(name_buf,*prog_path,25);*/

			truncate_path_to(*prog_path,name_buf,25);



/*sprintf(alert_text,"[1][path len %d|%s][ok]",strlen(name_buf),name_buf);
form_alert(1,alert_text);
*/			strcpy(temp_filename,*prog_path);
			set_tedinfo(dlog,VProg_Path,name_buf);
		}


		if(*working_dir==NULL)
		{
			sprintf(name_buf,"");
			sprintf(temp_working_dir,"");
			set_tedinfo(dlog,Working_Dir_Text,name_buf);
			set_button(dlog,Working_Dir_Box,Working_Program);		
		}
		else
		{
/*			stccpy(name_buf,*working_dir,25);*/
			truncate_path_to(*working_dir,name_buf,25);

			strcpy(temp_working_dir,*working_dir);
			set_tedinfo(dlog,Working_Dir_Text,name_buf);
			set_button(dlog,Working_Dir_Box,Working_Below);		
		}


		if(args==0)
		{
			set_tedinfo(dlog,Arg_Line1,arg1);
			set_tedinfo(dlog,Arg_Line2,arg2);
			set_tedinfo(dlog,Arg_Line3,arg3);
			set_button(dlog,Argument_Box,Argument_None);		
		}
		else
		{
			if(args==1)
			{
				set_button(dlog,Argument_Box,Argument_Ask);		
				set_tedinfo(dlog,Arg_Line1,arg1);
				set_tedinfo(dlog,Arg_Line2,arg2);
				set_tedinfo(dlog,Arg_Line3,arg3);
			}
			else
			{
				set_button(dlog,Argument_Box,Argument_Below);		
/*form_alert(1,"[1][copying arg][ok]");
*/				if(*args_string!=NULL)
				{
/*form_alert(1,"[1][copyinf ar][ok]");
*/					cp=*args_string;
					stccpy(arg1,cp,30);
/*					strmid(*working_dir,arg1,0,30);			
*/
					cp+=30;
					if(strlen(*args_string)>30)
						stccpy(arg2,cp,30);

/*					strmid(*working_dir,arg2,30,30);			
*/

					cp+=30;
					if(strlen(*args_string)>60)
						stccpy(arg3,cp,30);
/*					strmid(*working_dir,arg3,60,30);			
*/
					set_tedinfo(dlog,Arg_Line1,arg1);
					set_tedinfo(dlog,Arg_Line2,arg2);
					set_tedinfo(dlog,Arg_Line3,arg3);
				}
				else
				{
					set_tedinfo(dlog,Arg_Line1,arg1);
					set_tedinfo(dlog,Arg_Line2,arg2);
					set_tedinfo(dlog,Arg_Line3,arg3);
				}
			}
		}

	}


		draw_dialog(dlog);

	do{
/*		result=handle_dialog(dlog,0,0);
*/		result=form_do(dlog,0);
		dlog[result].ob_state&=~SELECTED;	/* de-select exit button */

		if(result==VProg_Path_FS)
		{
					fs_result=fsel_exinput(path,filename,&but,"LOCATE PROGRAM");
					remove_mask(path);
					if(but==1)
					{
							sprintf(temp_filename,"%s%s",path,filename);
						/*	stccpy(name_buf,temp_filename,25);*/
							truncate_path_to(temp_filename,name_buf,25);
							set_tedinfo(dlog,VProg_Path,name_buf);
					}
		}

		if(result==Working_Dir_FS)
		{
					fs_result=fsel_exinput(path,filename,&but,"LOCATE WORKING DIR");
					remove_mask(path);
					if(but==1)
					{
							sprintf(temp_working_dir,"%s",path);
							/*stccpy(name_buf,temp_working_dir,25);*/
							truncate_path_to(temp_filename,name_buf,25);
							set_tedinfo(dlog,Working_Dir_Text,name_buf);
					}
		}

		if(result==Argument_None || 
			result==Argument_Ask ||
			result==Argument_Below)
				set_button(dlog,Argument_Box,result);		

		if(result==Working_Program || 
			result==Working_Below)
				set_button(dlog,Working_Dir_Box,result);		

	/* if not cancel then */
	/* should object if progname is blank */

		if(result==VProg_OK)
		{
			get_tedinfo(dlog,VProg_Name,name_buf);
			cp=stpblk(name_buf);
			if(*cp=='\0')
			{
				form_alert(1,"[3][SORRY - Blank Names not allowed][ OK ]");
				result=-1;
			}
		}

	/* warning if pathname is blank */

	draw_dialog(dlog);
	}
	while(result!=VProg_OK && result!=VProg_CANCEL);

	not_draw_dialog(dlog);
	if(result==VProg_OK)
	{
		/* prog name */
		get_tedinfo(dlog,VProg_Name,prog_name);

		/* prog path */
		if(*prog_path==NULL)
		{
			/* allocate space to prog_path and copy temp_filename */
			*prog_path=(char *)malloc(FMSIZE+FNSIZE);
			if(*prog_path!=NULL)
			{
				strcpy(*prog_path,temp_filename);				
			}
		}
		else
			strcpy(*prog_path,temp_filename);

		/* args */
		args=get_button(dlog,Argument_Box);		

		if(args==Argument_None)
			args=0; 
		else
			if(args==Argument_Ask)
				args=1;
			else
				args=2;

		get_tedinfo(dlog,Arg_Line1,arg1);
		get_tedinfo(dlog,Arg_Line2,arg2);
		get_tedinfo(dlog,Arg_Line3,arg3);

/*		if(args!=*args_type)
		{

			/* handle the change in args */
*/
			if(args!=2 && *args_string!=NULL)
			{
				/* we did have some space ! */
				free(*args_string);
				*args_string=NULL;
			}	
			else
			{
				if(args==2 && *args_string==NULL)
				{
					/* we didn't have space but now we want it! */
					*args_string=(char *)malloc(91);
					if(*args_string!=NULL)
					{
						sprintf(*args_string,"%s%s%s",arg1,arg2,arg3);
					}
					else
					{
						args=0;
					}
				}
				else
				{
					if(args==2 && *args_string!=NULL)
					{
						/* we have space and we want it! */
							sprintf(*args_string,"%s%s%s",arg1,arg2,arg3);
					}
				}
			}
			
			*args_type=args;
/*		}
		else
		{
				if(args==2 && *args_string!=NULL)
				{
					/* we have space and we want it! */
						sprintf(*args_string,"%s%s%s",arg1,arg2,arg3);
				}
		}
*/
		/* working dir */
		wd=get_button(dlog,Working_Dir_Box);
		if(wd==Working_Program && *working_dir!=NULL)
		{
			/* move from having a path to not having a path */
			free(*working_dir);
			*working_dir=NULL;
		}
		else
			if(wd==Working_Below && *working_dir==NULL)
			{
				/* move from not having a path to having one */
				/* allocate space to prog_path and copy temp_filename */
				*working_dir=(char *)malloc(FMSIZE+FNSIZE);
				if(*working_dir!=NULL)
				{
					strcpy(*working_dir,temp_working_dir);				
				}
			}
			else
				if(wd==Working_Below && *working_dir!=NULL)
				{
					strcpy(*working_dir,temp_working_dir);				
				}
		
		

		return 1;
	}
	else
		return 0;
	
}


int create_document_for(
				char *doc,
				char **doc_path,
				char **doc_comment /*args string*/,
				char **package /*working_dir*/,
				char *execute_by,		/* 'D' for document! 'M' for Multitask, 'H' for Hog, 'E' for exit */
				long *date_created,
				long *current_version,
				long *date_last_edit,
				long *program_unique_no
)
{
	char name_buf[40];
	OBJECT *dlog;
	int result;
	int fs_result;
	int prog_item;
	short but;
	char *cp;
	char temp_filename[FNSIZE+FMSIZE];
	char temp_working_dir[FNSIZE+FMSIZE];
	char arg1[31],arg2[31],arg3[31];

	/*int wd;*/
	
	rsrc_gaddr(R_TREE,FORM_Create_Doc,&dlog);


	arg1[0]='\0';
	arg2[0]='\0';
	arg3[0]='\0';
	sprintf(temp_working_dir,"");
	sprintf(temp_filename,"");

	if(doc==NULL)
	{
		sprintf(name_buf,"");
		sprintf(temp_working_dir,"");
		sprintf(temp_filename,"");
		set_tedinfo(dlog,VD_Document_Name,name_buf);
		set_tedinfo(dlog,VD_Document_Path,name_buf);
		set_tedinfo(dlog,VD_Comment1,arg1);
		set_tedinfo(dlog,VD_Comment2,arg2);
		set_tedinfo(dlog,VD_Comment3,arg3);
/*		set_button(dlog,Argument_Box,Argument_None);		*/
		set_tedinfo(dlog,VD_Package_Path,temp_working_dir);
/*		set_button(dlog,Working_Dir_Box,Working_Program);	*/
		/* set date & time here */	

		if(*date_last_edit==0)
		{
			set_tedinfo(dlog,VD_Date_Edited,"Never");
		}
		else
		{
			/* unpack it */
			als_unpack_date(*date_last_edit,name_buf);
			set_tedinfo(dlog,VD_Date_Edited,name_buf);
		}

		if(*date_created==0)
		{
			set_tedinfo(dlog,VD_Date_Created,"Not");
		}
		else
		{
			/* unpack it */
			als_unpack_date(*date_created,name_buf);
			set_tedinfo(dlog,VD_Date_Created,name_buf);
		}

			sprintf(name_buf,"%ld",*current_version);
			set_tedinfo(dlog,VD_Version_No,name_buf);

	}
	else
	{
		if(doc==NULL)
		{
			sprintf(name_buf,"");
			set_tedinfo(dlog,VD_Document_Name,name_buf);
		}
		else
		{
			stccpy(name_buf,doc,31);
			set_tedinfo(dlog,VD_Document_Name,name_buf);
		}

		if(*doc_path==NULL)
		{
			sprintf(name_buf,"");
			sprintf(temp_filename,"");
			set_tedinfo(dlog,VD_Document_Path,name_buf);
		}
		else
		{
/*sprintf(alert_text,"[1][path len %d|%s][ok]",strlen(*doc_path),*doc_path);
form_alert(1,alert_text);
*/
			/*stccpy(name_buf,*doc_path,25);*/
			truncate_path_to(*doc_path,name_buf,25);


/*sprintf(alert_text,"[1][path len %d|%s][ok]",strlen(name_buf),name_buf);
form_alert(1,alert_text);
*/			strcpy(temp_filename,*doc_path);
			set_tedinfo(dlog,VD_Document_Path,name_buf);
		}


		if(*package==NULL)
		{
			sprintf(name_buf,"");
			sprintf(temp_working_dir,"");
			set_tedinfo(dlog,VD_Package_Path,name_buf);
		}
		else
		{

/*sprintf(alert_text,"[1][looking for package][ok]");
form_alert(1,alert_text);
*/
			if(*program_unique_no!=-1)
			{
/*sprintf(alert_text,"[1][package_unique %ld|index unique %ld][ok]",*program_unique_no,index_from_unique(*program_unique_no));
form_alert(1,alert_text);
sprintf(alert_text,"[1][package text %s][ok]",virtual_file_system[index_from_unique(*program_unique_no)].display_text);
form_alert(1,alert_text);
*/				stccpy(name_buf,virtual_file_system[index_from_unique(*program_unique_no)].display_text,25);
				strcpy(temp_working_dir,virtual_file_system[index_from_unique(*program_unique_no)].path);
				set_tedinfo(dlog,VD_Package_Path,name_buf);
			}
			else
			{
				/*stccpy(name_buf,*package,25);*/
				truncate_path_to(*package,name_buf,25);
				strcpy(temp_working_dir,*package);
				set_tedinfo(dlog,VD_Package_Path,name_buf);
			}
		}


				if(*doc_comment!=NULL)
				{
					cp=*doc_comment;
					stccpy(arg1,cp,30);

					cp+=30;
					if(strlen(*doc_comment)>30)
						stccpy(arg2,cp,30);

					cp+=30;
					if(strlen(*doc_comment)>60)
						stccpy(arg3,cp,30);
				}
					set_tedinfo(dlog,VD_Comment1,arg1);
					set_tedinfo(dlog,VD_Comment2,arg2);
					set_tedinfo(dlog,VD_Comment3,arg3);

		if(*date_last_edit==0)
		{
			set_tedinfo(dlog,VD_Date_Edited,"Never");
		}
		else
		{
			/* unpack it */
			als_unpack_date(*date_last_edit,name_buf);
			set_tedinfo(dlog,VD_Date_Edited,name_buf);
		}

		if(*date_created==0)
		{
			set_tedinfo(dlog,VD_Date_Created,"Not");
		}
		else
		{
			/* unpack it */
			als_unpack_date(*date_created,name_buf);
			set_tedinfo(dlog,VD_Date_Created,name_buf);
		}

		sprintf(name_buf,"%ld",*current_version);
		set_tedinfo(dlog,VD_Version_No,name_buf);
				
	
	}

		draw_dialog(dlog);

	do{
/*		result=handle_dialog(dlog,0,0);
*/		result=form_do(dlog,0);
		dlog[result].ob_state&=~SELECTED;	/* de-select exit button */

		if(result==VD_FS)
		{
					fs_result=fsel_exinput(path,filename,&but,"LOCATE DOCUMENT");
					remove_mask(path);
					if(but==1)
					{
							sprintf(temp_filename,"%s%s",path,filename);
							/*stccpy(name_buf,temp_filename,25);*/
							truncate_path_to(temp_filename,name_buf,25);
							set_tedinfo(dlog,VD_Document_Path,name_buf);
					}
		}


		if(result==VD_Package_FS)
		{
					fs_result=fsel_exinput(path,filename,&but,"LOCATE PACKAGE");
					remove_mask(path);
					if(but==1)
					{
							sprintf(temp_working_dir,"%s%s",path,filename);
							/*stccpy(name_buf,temp_working_dir,25);*/
							truncate_path_to(temp_working_dir,name_buf,25);
							set_tedinfo(dlog,VD_Package_Path,name_buf);
					}
		}


	/* if not cancel then */
	/* should object if progname is blank */

		if(result==VD_OK)
		{
			get_tedinfo(dlog,VD_Document_Name,name_buf);
			cp=stpblk(name_buf);
			if(*cp=='\0')
			{
				form_alert(1,"[3][SORRY - Blank Names not allowed][ OK ]");
				result=-1;
			}
		}

	/* warning if pathname is blank */

	draw_dialog(dlog);
	}
	while(result!=VD_OK && result!=VD_CANCEL);

	not_draw_dialog(dlog);
	if(result==VD_OK)
	{
		/* doc name */
		get_tedinfo(dlog,VD_Document_Name,doc);

		/* doc path */
		if(*doc_path==NULL)
		{
			/* allocate space to doc_path and copy temp_filename */
			*doc_path=(char *)malloc(FMSIZE+FNSIZE);
			if(*doc_path!=NULL)
			{
				strcpy(*doc_path,temp_filename);				
			}
		}
		else
			strcpy(*doc_path,temp_filename);


		get_tedinfo(dlog,VD_Comment1,arg1);
		get_tedinfo(dlog,VD_Comment2,arg2);
		get_tedinfo(dlog,VD_Comment3,arg3);

/*		if(args!=*args_type)
		{

			/* handle the change in args */
*/
			if(arg1[0]=='\0' && arg2[0]=='\0' && arg3[0]=='\0' && *doc_comment!=NULL)
			{
				/* we did have some space ! */
				free(*doc_comment);
				*doc_comment=NULL;
			}	
			else
			{
				if((arg1[0]!='\0' || arg2[0]!='\0' || arg3[0]!='\0') && *doc_comment==NULL)
				{
					/* we didn't have space but now we want it! */
					*doc_comment=(char *)malloc(91);
					if(*doc_comment!=NULL)
					{
						sprintf(*doc_comment,"%s%s%s",arg1,arg2,arg3);
					}
				}
				else
				{
					if((arg1[0]!='\0' || arg2[0]!='\0' || arg3[0]!='\0') && *doc_comment!=NULL)
					{
						/* we have space and we want it! */
							sprintf(*doc_comment,"%s%s%s",arg1,arg2,arg3);
					}
				}
			}
			
		/* package path dir */
		
		if(temp_working_dir[0]!='\0')
		{
			/* search for that path in the packages */
			prog_item=unique_from_path(temp_working_dir);

			if(prog_item!=*program_unique_no)
			{
				/* if found... ask, do you want to use symbolic link? */
				if(prog_item!=-1)
				{
					result=form_alert(1,"[1][This package has been defined|would you like a symbolic link?][Yes|No]");
	
					/* if yes then add symbolic link */
	
					if(result==1)
					{
						*program_unique_no=prog_item;
						strcpy(temp_working_dir,virtual_file_system[prog_item].display_text);
					}
				}
			}
	
			if(temp_working_dir[0]=='\0' && *package!=NULL)
			{
				/* move from having a path to not having a path */
				free(*package);
				*package=NULL;
			}
			else
				if(temp_working_dir[0]!='\0' && *package==NULL)
				{
					/* move from not having a path to having one */
					/* allocate space to package and copy temp_working_dir */
					*package=(char *)malloc(FMSIZE+FNSIZE);
					if(*package!=NULL)
					{
						strcpy(*package,temp_working_dir);				
					}
				}
				else
					if(temp_working_dir[0]!='\0' && *package!=NULL)
					{
						strcpy(*package,temp_working_dir);				
					}
		}	
		

		return 1;
	}
	else
		return 0;
	
}



