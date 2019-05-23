/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   op.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myaremen <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/04/25 15:57:21 by myaremen          #+#    #+#             */
/*   Updated: 2019/04/26 07:54:57 by oivanyts         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "vm.h"

uint32_t threads_alive(t_threads *threads)
{
	size_t thread;
	uint32_t alive;

	alive = 0;
	thread = 0;
	while (thread < threads_size(threads))
	{
		if (threads_at(threads, thread)->alive)
		{
			alive += threads_at(threads, thread)->lives;
		}
		++thread;
	}
	return (alive);
}

void    kill_thread_if_no_lives(t_thread *th)
{
	if (th->lives == 0)
	{
		th->alive = 0;
	}
	else
	{
		th->lives = 0;
	}
}

void    foreach_thread(t_threads *threads, void(*func)(t_thread*))
{
	int32_t thread;

	thread = threads_size(threads) - 1;
	while (thread >= 0)
	{
		func(threads_at(threads, thread));
		--thread;
	}
}

void	vm_cycle(t_vm *vm)
{
	int32_t cycles;
	int32_t cycles_to_die;
	uint32_t checks;
	uint32_t alive;

	cycles = 1;
	cycles_to_die = CYCLE_TO_DIE;
	checks = 0;
	alive = threads_alive(&vm->threads);
	while (alive || cycles_to_die > 0)
	{
		ft_printf("It is now cycle %d\n", vm->cycle);
		if (cycles == cycles_to_die)
		{
			alive = threads_alive(&vm->threads);
			foreach_thread(&vm->threads, kill_thread_if_no_lives);
			cycles = 0;
			++checks;
			if (alive >= NBR_LIVE)
			{
				cycles_to_die -= CYCLE_DELTA;
				ft_printf("Cycle to die is now %d\n", cycles_to_die);
				checks = 0;
			}
		}
		if (checks == MAX_CHECKS)
		{
			cycles_to_die -= CYCLE_DELTA;
			ft_printf("Cycle to die is now %d\n", cycles_to_die);
			checks = 0;
		}
		/*if (get_vm(0)->cycle == 2111)
		{
			get_vm(0);
		}*/
		foreach_thread(&vm->threads, op_exec);
		if (vm->cycle == vm->o_dump_point && vm->o_dump)
		{
//			poor_mans_visualization(((t_thread *)(players->threads.arr.arr))->vm_memory, get_vm(0)->players, nplayers);
			return ;
		}
		++cycles;
		++vm->cycle;
//		poor_mans_visualization(((t_thread *)(players->threads.arr.arr))->vm_memory, players, nplayers);
		//ft_printf("\ncycle = %u ended\n", global_cycles);
	}
}

uint8_t decode_opcode(struct s_thread *pc)
{
	uint8_t opcode;

	opcode = as_byte(pc->vm_memory)[pc->ip];
	pc->ip += 1;
	pc->ip %= MEM_SIZE;
	return (opcode);
}

bool	check_op_params(uint8_t opcode, uint8_t tparams)
{
	if (op_tab[opcode].targs[0] && (op_tab[opcode].targs[0] & ((tparams >> 6) & 0x3)) == 0)
	{
		return (0);
	}
	else if (op_tab[opcode].targs[1] && (op_tab[opcode].targs[1] & ((tparams  >> 4) & 0x3)) == 0)
	{
		return (0);
	}
	else if (op_tab[opcode].targs[2] && (op_tab[opcode].targs[2] & ((tparams >> 2) & 0x3)) == 0)
	{
		return (0);
	}
	else
	{
		return (1);
	}
}

uint8_t	decode_tparams(struct s_thread *pc, uint8_t opcode)
{
	uint8_t tparams;

	if (op_tab[opcode].codoctal)
	{
		tparams = as_byte(pc->vm_memory)[pc->ip];
		if (!check_op_params(opcode, tparams))
		{
			pc->op.valid = 0;
		}
		pc->ip += 1;
		pc->ip %= MEM_SIZE;
	}
	else
	{
		tparams = (op_tab[opcode].targs[0] << 6) | (op_tab[opcode].targs[1] << 4) | (op_tab[opcode].targs[2] << 2);
	}
	return (tparams);
}

uint32_t swap32(uint32_t toswap)
{
	return (toswap >> 24 | ((toswap >> 8) & 0xff00 ) | ((toswap << 8) & 0xff0000 ) | toswap << 24);
}


uint16_t swap16(uint16_t toswap)
{
	return (toswap >> 8 | toswap << 8);
}

uint8_t get_param_type(uint8_t tparams, uint8_t param_number)
{
	if (param_number == 1)
	{
		return ((tparams & T_FIRST_PARAM) >> 6) & 0x3;
	}
	else if (param_number == 2)
	{
		return ((tparams & T_SECOND_PARAM) >> 4) & 0x3;
	}
	else
	{
		return ((tparams & T_THIRD_PARAM) >> 2) & 0x3;
	}
}

t_memory decode_param(t_decoded_op op, t_thread *pc, uint8_t param_number)
{
	t_memory param;
	uint8_t  tparam;

	memory_init(&param, 0, 0);
	if (param_number <= op_tab[op.opcode].args)
	{
		tparam = get_param_type(pc->op.tparams, param_number);
		if (tparam == REG_CODE)
		{
			if (as_byte(pc->vm_memory)[pc->ip] > REG_NUMBER || as_byte(pc->vm_memory)[pc->ip] == 0)
			{
				pc->op.valid = 0;
			}
			memory_init(&param, &(pc->vm_memory)[pc->ip], T_REG);
			pc->ip += 1;
		}
		else if (tparam == DIR_CODE)
		{
			if (op_tab[op.opcode].tdir_size == 0)
			{
				memory_init(&param,  &pc->vm_memory[pc->ip], DIR_SIZE);
				pc->ip += DIR_SIZE;
			}
			else
			{
				memory_init(&param,  &pc->vm_memory[pc->ip], IND_SIZE);
				pc->ip += IND_SIZE;
			}
		}
		else if (tparam == IND_CODE)
		{
			memory_init(&param, &pc->vm_memory[pc->ip], T_IND);
			pc->ip += IND_SIZE;
		}
		else
		{
			pc->op.valid = 0;
		}
		pc->ip %= MEM_SIZE;
	}
	return (param);
}

void	op_decode(t_thread *pc)
{
	if (pc->op.valid == 0)
	{
		return ;
	}
	pc->op.tparams = decode_tparams(pc, pc->op.opcode);
	pc->op.args[0] = decode_param(pc->op, pc, 1);
	pc->op.args[1] = decode_param(pc->op, pc, 2);
	pc->op.args[2] = decode_param(pc->op, pc, 3);
}

void print_moves(const t_thread *pc)
{
	int8_t	i = 0;
	int8_t funcsize;
	if (pc->op.opcode == 8 && pc->cf)
	{
		return;
	}
	else
	{
		funcsize = pc->ip % MEM_SIZE - pc->op.ip % MEM_SIZE;
	}
	ft_printf("ADV %d (0x%0.4x -> 0x%0.4x)", funcsize, pc->op.ip % MEM_SIZE, pc->ip % MEM_SIZE);
	while (i < funcsize)
	{
		ft_printf(" %0.2x", (uint8_t)pc->vm_memory[(pc->op.ip + i) % MEM_SIZE]);
		i++;
	}
	ft_printf(" \n", op_tab[pc->op.opcode].name);
	//ft_printf(" {red}[%s]{eoc}\n", op_tab[pc->op.opcode].name);

}

void	print_op(const t_thread *pc)
{
	ft_printf("P    %d | %s\n", pc - threads_at(&get_vm(0)->threads, 0) + 1, op_tab[pc->op.opcode].name);
}

void	op_exec(t_thread *pc)
{
	if (pc->processing == 0)
	{
//		poor_mans_visualization(pc->vm_memory, pc->player, 1);
		pc->op.valid = 1;
		pc->op.ip = pc->ip;
		pc->op.opcode = as_byte(pc->vm_memory)[pc->ip];
		if (pc->op.opcode < 1 || pc->op.opcode > 16)
		{
			pc->op.valid = 0;
			pc->wait = 1;
		}
		else
		{
			pc->wait = op_tab[pc->op.opcode - 1].cycle;
		}
		pc->op.opcode -= 1;
		pc->processing = 1;
	}
	if (pc->wait)
	{
	    pc->wait -= 1;
	    if (pc->wait)
		{
	    	return ;
		}
	}
	if (pc->alive == 0)
	{
		return ;
	}
	decode_opcode(pc);
	op_decode(pc);
	if (pc->op.valid)
	{
		//print_op(pc);
		pc->processing = 0;
		int pc_i = pc - threads_at(&get_vm(0)->threads, 0);
		opcalls[pc->op.opcode].opfunc(pc, &pc->op.args[0], &pc->op.args[1], &pc->op.args[2]); //dalshe zatiraetsa adres pc, poetomu sohranyaem ego v pc_i
		if (get_vm(0)->o_dump)
			print_moves(threads_at(&get_vm(0)->threads, pc_i));
		//poor_mans_visualization(pc->vm_memory, get_vm(0)->players, get_vm(0)->nplayers);
	}
}

t_vm *get_vm(t_vm *vm)
{
	static t_vm *m_vm = 0;

	if (m_vm == 0)
	{
		m_vm = vm;
	}
	return (m_vm);
}

t_op    op_tab[17] =
				{
						{"live", 1, {T_DIR}, 1, 10, "alive", 0, 0},
						{"ld", 2, {T_DIR | T_IND, T_REG}, 2, 5, "load", 1, 0},
						{"st", 2, {T_REG, T_IND | T_REG}, 3, 5, "store", 1, 0},
						{"add", 3, {T_REG, T_REG, T_REG}, 4, 10, "addition", 1, 0},
						{"sub", 3, {T_REG, T_REG, T_REG}, 5, 10, "soustraction", 1, 0},
						{"and", 3, {T_REG | T_DIR | T_IND, T_REG | T_IND | T_DIR, T_REG}, 6, 6,
								"et (and  r1, r2, r3   r1&r2 -> r3", 1, 0},
						{"or", 3, {T_REG | T_IND | T_DIR, T_REG | T_IND | T_DIR, T_REG}, 7, 6,
								"ou  (or   r1, r2, r3   r1 | r2 -> r3", 1, 0},
						{"xor", 3, {T_REG | T_IND | T_DIR, T_REG | T_IND | T_DIR, T_REG}, 8, 6,
								"ou (xor  r1, r2, r3   r1^r2 -> r3", 1, 0},
						{"zjmp", 1, {T_DIR}, 9, 20, "jump if zero", 0, 1},
						{"ldi", 3, {T_REG | T_DIR | T_IND, T_DIR | T_REG, T_REG}, 10, 25,
								"load index", 1, 1},
						{"sti", 3, {T_REG, T_REG | T_DIR | T_IND, T_DIR | T_REG}, 11, 25,
								"store index", 1, 1},
						{"fork", 1, {T_DIR}, 12, 800, "fork", 0, 1},
						{"lld", 2, {T_DIR | T_IND, T_REG}, 13, 10, "long load", 1, 0},
						{"lldi", 3, {T_REG | T_DIR | T_IND, T_DIR | T_REG, T_REG}, 14, 50,
								"long load index", 1, 1},
						{"lfork", 1, {T_DIR}, 15, 1000, "long fork", 0, 1},
						{"aff", 1, {T_REG}, 16, 2, "aff", 1, 0},
						{0, 0, {0}, 0, 0, 0, 0, 0}
				};