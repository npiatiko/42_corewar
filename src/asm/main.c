/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: npiatiko <npiatiko@student.unit.ua>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/04/25 11:47:27 by npiatiko          #+#    #+#             */
/*   Updated: 2019/04/25 15:16:10 by npiatiko         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "asm.h"



int		main(int argc, char **argv)
{
	t_token_list	*lst;

	lst = ft_memalloc(sizeof(t_token_list));
	lst->next = ft_memalloc(sizeof(t_token_list));

	ft_printf("%d", ft_listlen(lst));
//	ft_printf("%s %d %d %d %d %s %d %d\n", g_op_tab[0].name, g_op_tab[0].args, g_op_tab[0].targs[0], g_op_tab[0].opcode, g_op_tab[0].cycle, g_op_tab[0].descr, g_op_tab[0].codoctal, g_op_tab[0].codoctal);
	return (0);
}
