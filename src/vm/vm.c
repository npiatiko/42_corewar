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
	if (th->lives == 0 && th->alive)
	{
		th->alive = 0;
		get_vm(0)->options.o_v_param & 8 ?
		ft_printf("Process %d hasn't lived for %d cycles (CTD %d)\n",
				th - threads_at(&get_vm(0)->threads, 0) + 1,
				get_vm(0)->cycle - th->last_live, get_vm(0)->ctd) : 0;
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
	uint32_t checks;
	uint32_t alive;

	cycles = 1;
	vm->ctd = CYCLE_TO_DIE;
	checks = 0;
	alive = 1;
	while (alive)
	{
		!vm->options.visual_ncurses && (vm->options.o_v_param & 2) ? ft_printf("It is now cycle %d\n", vm->cycle , vm->ctd, cycles) : 0;
		foreach_thread(&vm->threads, op_exec);
		if (cycles >= vm->ctd)
		{
			alive = threads_alive(&vm->threads);
			foreach_thread(&vm->threads, kill_thread_if_no_lives);
			cycles = 0;
			++checks;
			if (alive >= NBR_LIVE)
			{
				vm->ctd -= CYCLE_DELTA;
				!vm->options.visual_ncurses && (vm->options.o_v_param & 2) ? ft_printf("Cycle to die is now %d\n", vm->ctd) : 0;
				checks = 0;
			}
		}
		if (checks == MAX_CHECKS)
		{
			vm->ctd -= CYCLE_DELTA;
			!vm->options.visual_ncurses && (vm->options.o_v_param & 2) ? ft_printf("Cycle to die is now %d\n", vm->ctd) : 0;
			checks = 0;
		}
		if (vm->cycle == vm->options.o_dump_point && vm->options.o_dump)
		{
			poor_mans_visualization(threads_at(&vm->threads, 0)->vm_memory);
			return ;
		}
		if (vm->ctd < CYCLE_TO_DIE % CYCLE_DELTA - CYCLE_DELTA)
		{
			return ;
		}
		++cycles;
		++vm->cycle;
	}
}

void decode_opcode(struct s_thread *pc)
{
	pc->ip += 1;
	pc->ip %= MEM_SIZE;
}

uint8_t get_param_type(uint8_t opcode, uint8_t tparams, uint8_t param_number)
{
	if (param_number == 1)
	{
		if ((op_tab[opcode].targs[param_number - 1] & T_IND) && (((tparams & T_FIRST_PARAM) >> 6) & 0x3) == IND_CODE)
		{
			return (T_IND);
		}
		else if ((op_tab[opcode].targs[param_number - 1] & T_REG) && (((tparams & T_FIRST_PARAM) >> 6) & 0x3) == REG_CODE)
		{
			return (T_REG);
		}
		else if ((op_tab[opcode].targs[param_number - 1] & T_DIR) && (((tparams & T_FIRST_PARAM) >> 6) & 0x3) == DIR_CODE)
		{
			return (T_DIR);
		}
		else
		{
			return (0);
		}
	}
	else if (param_number == 2)
	{
		if ((op_tab[opcode].targs[param_number - 1] & T_IND) && (((tparams & T_SECOND_PARAM) >> 4) & 0x3) == IND_CODE)
		{
			return (T_IND);
		}
		else if ((op_tab[opcode].targs[param_number - 1] & T_REG) && (((tparams & T_SECOND_PARAM) >> 4) & 0x3) == REG_CODE)
		{
			return (T_REG);
		}
		else if ((op_tab[opcode].targs[param_number - 1] & T_DIR) && (((tparams & T_SECOND_PARAM) >> 4) & 0x3) == DIR_CODE)
		{
			return (T_DIR);
		}
		else
		{
			return (0);
		}
	}
	else
	{
		if ((op_tab[opcode].targs[param_number - 1] & T_IND) && (((tparams & T_THIRD_PARAM) >> 2) & 0x3) == IND_CODE)
		{
			return (T_IND);
		}
		else if ((op_tab[opcode].targs[param_number - 1] & T_REG) && (((tparams & T_THIRD_PARAM) >> 2) & 0x3) == REG_CODE)
		{
			return (T_REG);
		}
		else if ((op_tab[opcode].targs[param_number - 1] & T_DIR) && (((tparams & T_THIRD_PARAM) >> 2) & 0x3) == DIR_CODE)
		{
			return (T_DIR);
		}
		else
		{
			return (0);
		}
	}
}

uint8_t force_get_param_type(uint8_t tparams, uint8_t param_number)
{
	if (param_number == 1)
	{
		if ((((tparams & T_FIRST_PARAM) >> 6) & 0x3) == IND_CODE)
		{
			return (T_IND);
		}
		else if ((((tparams & T_FIRST_PARAM) >> 6) & 0x3) == REG_CODE)
		{
			return (T_REG);
		}
		else if ((((tparams & T_FIRST_PARAM) >> 6) & 0x3) == DIR_CODE)
		{
			return (T_DIR);
		}
		else
		{
			return (0);
		}
	}
	else if (param_number == 2)
	{
		if ((((tparams & T_SECOND_PARAM) >> 4) & 0x3) == IND_CODE)
		{
			return (T_IND);
		}
		else if ((((tparams & T_SECOND_PARAM) >> 4) & 0x3) == REG_CODE)
		{
			return (T_REG);
		}
		else if ((((tparams & T_SECOND_PARAM) >> 4) & 0x3) == DIR_CODE)
		{
			return (T_DIR);
		}
		else
		{
			return (0);
		}
	}
	else
	{
		if ((((tparams & T_THIRD_PARAM) >> 2) & 0x3) == IND_CODE)
		{
			return (T_IND);
		}
		else if ((((tparams & T_THIRD_PARAM) >> 2) & 0x3) == REG_CODE)
		{
			return (T_REG);
		}
		else if ((((tparams & T_THIRD_PARAM) >> 2) & 0x3) == DIR_CODE)
		{
			return (T_DIR);
		}
		else
		{
			return (0);
		}
	}
}

bool	check_op_params(uint8_t opcode, uint8_t tparams)
{
	if (op_tab[opcode].targs[0] && get_param_type(opcode, tparams, 1) == 0)
	{
		return (0);
	}
	else if (op_tab[opcode].targs[1] && get_param_type(opcode, tparams, 2) == 0)
	{
		return (0);
	}
	else if (op_tab[opcode].targs[2] && get_param_type(opcode, tparams, 3) == 0)
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

	tparams = 0;
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
		if (op_tab[opcode].targs[0] & T_REG)
		{
			tparams |= (REG_CODE << 6);
		}
		if (op_tab[opcode].targs[0] & T_DIR)
		{
			tparams |= (DIR_CODE << 6);
		}
		if (op_tab[opcode].targs[0] & T_IND)
		{
			tparams |= (IND_CODE << 6);
		}
		if (op_tab[opcode].targs[1] & T_REG)
		{
			tparams |= (REG_CODE << 4);
		}
		if (op_tab[opcode].targs[1] & T_DIR)
		{
			tparams |= (DIR_CODE << 4);
		}
		if (op_tab[opcode].targs[1] & T_IND)
		{
			tparams |= (IND_CODE << 4);
		}
		if (op_tab[opcode].targs[2] & T_REG)
		{
			tparams |= (REG_CODE << 2);
		}
		if (op_tab[opcode].targs[2] & T_DIR)
		{
			tparams |= (DIR_CODE << 2);
		}
		if (op_tab[opcode].targs[2] & T_IND)
		{
			tparams |= (IND_CODE << 2);
		}
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

t_memory decode_param(t_decoded_op op, t_thread *pc, uint8_t param_number)
{
	t_memory param;
	uint8_t  tparam;

	memory_init(&param, 0, 0);
	if (param_number <= op_tab[op.opcode].args)
	{
		tparam = force_get_param_type(pc->op.tparams, param_number);
		if (tparam == T_REG)
		{
			if (as_byte(pc->vm_memory)[pc->ip] > REG_NUMBER || as_byte(pc->vm_memory)[pc->ip] == 0)
			{
				pc->op.valid = 0;
			}
			memory_init(&param, &(pc->vm_memory)[pc->ip], T_REG);
			memory_set_bounds(&param, param.memory, param.memory + 8);
			pc->ip += 1;
		}
		else if (tparam == T_DIR)
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
			memory_set_bounds(&param, pc->vm_memory, pc->vm_memory + MEM_SIZE);
		}
		else if (tparam == T_IND)
		{
			memory_init(&param, &pc->vm_memory[pc->ip], IND_SIZE);
			memory_set_bounds(&param, pc->vm_memory, pc->vm_memory + MEM_SIZE);
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
	if ((pc->op.opcode == 8 && pc->cf) || (pc->op.opcode < 0 || pc->op.opcode > 15))
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

uint32_t get_thread_number(const t_thread *pc)
{
	uint32_t pc_i;

	pc_i = pc - threads_at(&get_vm(0)->threads, 0);
	return (pc_i);
}

void	op_exec(t_thread *pc)
{
	if (pc->processing == 0)
	{
		ft_memset(&pc->op, 0, sizeof(t_decoded_op));
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
	pc->processing = 0;
	int pc_i = get_thread_number(pc);
	if (pc->op.valid)
	{
		if (get_vm(0)->options.visual_ncurses == 0 && get_vm(0)->options.o_op)
		{
			print_op(pc);
		}
		opcalls[pc->op.opcode].opfunc(pc, &pc->op.args[0], &pc->op.args[1], &pc->op.args[2]);
	}
	!get_vm(0)->options.visual_ncurses && get_vm(0)->options.o_v_param & 16 ? print_moves(threads_at(&get_vm(0)->threads, pc_i)) : 0;
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
