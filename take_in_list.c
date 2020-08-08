/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   take_in_list.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dmarsell <dmarsell@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/08/03 01:03:54 by dmarsell          #+#    #+#             */
/*   Updated: 2020/08/08 04:07:11 by dmarsell         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ls.h"

void    ft_free(t_list *nm)
{
    t_list  *head;

    while (nm)
    {
        nm->child ? ft_free(nm->child) : 1;
        nm->dir ? free(nm->dir) : 1;
        nm->name ? free(nm->name) : 1;
        nm->path ? free(nm->path) : 1;
        nm->link ? free(nm->link) : 1;
        nm->pwd ? free(nm->pwd) : 1;
        nm->grp ? free(nm->grp) : 1;
        head = nm;
        nm = nm->next;
        free(head);
    }
}

void            ft_total(t_list *nm, int flags)
{
    unsigned int res;

    res = 0;
    while(nm)
    {
        if (!(flags & FLAG_A) && nm->name[0] == '.')
        {
            nm = nm->next;
            continue ;
        }   
        res += nm->stat.st_blocks;
        nm = nm->next;
    }
    ft_printf("total %u\n", res);
}

void 			print_list(t_list *nm, int flags)
{
	while(nm)
	{
		if ((flags ^ FLAG_A) && nm->name[0] == '.')
			nm = nm->next;
		else
		{
			ft_printf("%s\n", nm->name);
			nm = nm->next;
		}
	}
}

void           big_str(t_list *nm, int flags)
{
    t_opt       *lst;

    about_file(nm);
    lst = parse_big(nm);
    while(nm)
    {
		if (!(flags & FLAG_A) && nm->name[0] == '.')
			nm = nm->next;
		else {
			nm->format = type_file(nm);
			file_mode(nm->stat.st_mode);
			format_num(nm->stat.st_nlink, lst->olink);
			format_str(nm->pwd, lst->opwd);
            write(1, "  ", 2);
			format_str(nm->grp, lst->ogrp);
			format_min_size(nm, lst);
			format_maj_size(nm, lst);
			format_time(nm->stat.st_mtimespec.tv_sec);
			ft_putstr(nm->name);
			soft_link(nm->path);
			write(1, "\n", 1);
			nm = nm->next;
		}
    }
    // write(1, "\n", 1);
    free(lst);
}

void 			begin_of_list(t_list *nm)
{
	t_list  *cur;

	cur = nm;
	ft_printf("%s:\n", nm->dir);
}


int            print(t_list *nm, int flags)
{
    int     i;
    t_list  *cur;

    if (nm == NULL)
        return (1);
    i = 0;
    cur = nm;
    if (lstat(cur->path, &cur->stat) < 0)
        return (ft_printf("./ft_ls: %s: No such file or directory\n", cur->name));
    flags & FLAG_ARG ? begin_of_list(nm) : 1;
    !(flags & FLAG_ARG) && (flags & FLAG_R) ? flags ^= FLAG_ARG : 1;
    flags & FLAG_L ? ft_total(cur, flags) : 1;
    if (flags & FLAG_L)
    	big_str(cur, flags);
	else
		print_list(nm, flags);
    while(nm)
    {
        if(nm->child)
        {
            if (!(flags & FLAG_A) && nm->name[0] == '.')
            {
                nm = nm->next;
                continue ;
            }   
            write(1, "\n", 1);
            print(nm->child, flags);
        }
        nm = nm->next;
    }
    return (0);
}

int         are_you_dir(t_list *dir)
{
    t_list      *new_dir;

    if (S_ISDIR(dir->stat.st_mode))
    {
        if (!(new_dir = (t_list *)malloc(sizeof(t_list))))
            ft_perror("are_you_dir() malloc: ", dir);
        ft_null(new_dir);
        dir->child = new_dir;
        new_dir->path = dir->path;
        return (1);
    }
    dir->child = NULL;
    return (0);
}

void        process(t_list *cur, char *name, char *way, int flags)
{
    cur->name = ft_strdup(name);
    cur->path = slash_strjoin(way, cur->name);
    cur->dir = ft_strdup(way);
    if(lstat(cur->path, &cur->stat) < 0)
        ft_perror("process() lstat: ", cur);
    if ((flags & FLAG_R) && (ft_strcmp(cur->name, ".") != 0 ) && (ft_strcmp(cur->name, "..") != 0))
    {
        if (are_you_dir(cur))
            cur->child = in_directory(cur->path, cur->child, flags);
    }
}

t_list     *in_directory(char *way, t_list *names, int flags)
{
    struct dirent   *entry;
    DIR             *dirp;
    t_list          *cur;

    if (!(dirp = opendir(way)))
        return (ft_start_file(way, names));
    if (!(entry = readdir(dirp)))
        ft_perror("in_directory() readdir: ", names);
    cur = names;
    while (entry)
    {
        process(cur, entry->d_name, way, flags);
        if ((entry = readdir(dirp)))
        {
            if (!(cur->next = (t_list *)malloc(sizeof(t_list))))
                ft_perror("in_directory() malloc: ", cur);
            cur = cur->next;
            ft_null(cur);
        }
    }
    cur->next = NULL;
    if (closedir(dirp) < 0)
        ft_perror("in_directory() closedir: ", cur);
    return (names);
}

int    ft_start(int flags, char *way)
{
    t_list  *names;
    t_list  *go;
    
    if (way == NULL)
        return (-1);
    if (!(names = (t_list *)malloc(sizeof(t_list))))
        ft_perror("ft_start() malloc: ", names);
    ft_null(names);
    go = in_directory(way, names, flags);
    go = sorting(go, flags);
    print(go, flags);
    ft_free(go);
    return (0);
}

void    ft_init(t_head *head, t_crutch *data, int file)
{
    if (file == VALID_ARG_FILE)
    {
        if (!(head->val_file_start = (t_list *)malloc(sizeof(t_list))))
            ft_perror("ft_init() malloc: ", NULL);
        ft_null(head->val_file_start);
        head->val_file_start->name = ft_findlastname(data->way);
        head->val_file_start->path = ft_strdup(data->way);
    }
    else if (file == VALID_ARG_DIR)
    {
        if (!(head->val_dir_start = (t_list *)malloc(sizeof(t_list))))
            ft_perror("ft_init() malloc: ", NULL);
        ft_null(head->val_dir_start);
        head->val_dir_start->name = ft_findlastname(data->way);
        head->val_dir_start->path = ft_strdup(data->way);
    }
    else if (file == INVALID_ARG)
    {
        if (!(head->invalid_start = (t_list *)malloc(sizeof(t_list))))
            ft_perror("ft_init() malloc: ", NULL);
        ft_null(head->invalid_start);
        head->invalid_start->name = ft_findlastname(data->way);
        head->invalid_start->path = ft_strdup(data->way);
    }
}

void    ft_prestart(t_head *head, char **argv, t_crutch *data)
{
    t_list          *argp;
    t_list          *failp;
    int             notfile;
    int             argument;
    
    argument = 0;
    notfile = 0;
    while(argv[data->count])
    {
        data->way = ft_parsing(argv, data->way, &data->flags, &data->count);
        head->valid = lstat(data->way, &head->stat) ? 1 : 0;   
        if (head->valid == 0)
        {
            if (notfile == 0)
            { 
                ft_init(head, data, INVALID_ARG);
                failp = head->invalid_start;
                notfile++;
                continue ;
            }
            failp = ft_invalid_create(data, failp);
        }
        else if (S_ISDIR(head->stat.st_mode))
        {
            if (argument == 0)
            {
                ft_init(head, data, VALID_ARG_DIR);
                argp = head->val_file_start;
                argument++;
                continue ;
            }
            argp = ft_dir_create(data, argp);
            argument++;
        }
        // else if (S_ISDIR(head->stat.st_mode) == 0)
        // {
        //     if (argument == 0)
        //     {
        //         ft_init(head, data, VALID_ARG_FILE);
        //         argp = head->val_file_start;
        //         argument++;
        //         continue ;
        //     }
        //     argp = ft_file_create(data, argp);
        //     argument++;
        // }
    }
    if (argument)
        argp->next = NULL;
    if (notfile)
        failp->next = NULL;
    argument > 1 ? data->flags |= FLAG_ARG : 1;
}

int     main(int argc, char **argv)
{
    t_head          head;
    t_crutch        data;
    t_list          *argp;
    t_list          *failp;
    
    data.arg = 0;
    data.fail= 0;
    data.flags = 0;
    data.count = 1;
    if (argc == 1)
    {
        data.way = "./";
        data.flags = FLAG_NON;
        ft_start(data.flags, data.way);
        return (0);
    }
    ft_prestart(&head, argv, &data);
    failp = sorting(head.invalid_start, data.flags);
    while(failp)
    {
        print(failp, data.flags);
        failp = failp->next;
    }
    ft_free(head.invalid_start);
    argp = sorting(head.val_file_start, data.flags);
    while(argp)
    {
        ft_start(data.flags, argp->path);
        argp->next ? write(1, "\n", 1) : 1;
        argp = argp->next;
    }
    ft_free(head.val_file_start);
    return (0);
}
