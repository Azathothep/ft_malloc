#ifndef LST_FREE_H
# define LST_FREE_H

void	*lst_free_add(t_free **begin_lst, int size, void *addr);
void	lst_free_remove(t_free **begin_lst, t_free *slot);

#endif
