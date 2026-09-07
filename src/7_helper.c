/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7_helper.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mennih < mennih@student.1337.ma>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 14:24:47 by mennih            #+#    #+#             */
/*   Updated: 2026/09/07 02:04:40 by mennih           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	put_dongles(t_coder *c)
{
	dongle_release(c->sim, c);
}

bool	coder_wait_grant(t_sim *sim, t_coder *c)
{
	bool	got;

	pthread_mutex_lock(&c->cond_mutex);
	while (!c->granted && !sim_is_stopped(sim))
		pthread_cond_wait(&c->cond, &c->cond_mutex);
	got = c->granted;
	pthread_mutex_unlock(&c->cond_mutex);
	return (got);
}

long long	next_cooldown_us(t_sim *sim)
{
	long long	now;
	long long	remain;
	long long	nearest;
	int			i;

	now = get_time_ms();
	nearest = 10 * 1000LL;
	pthread_mutex_lock(&sim->arb_mutex);
	i = 0;
	while (i < sim->n)
	{
		if (!sim->dongles[i].in_use && sim->dongles[i].release_time != 0)
		{
			remain = sim->dongles[i].release_time
				+ sim->dongle_cooldown - now;
			if (remain > 0 && remain * 1000LL < nearest)
				nearest = remain * 1000LL;
		}
		i++;
	}
	pthread_mutex_unlock(&sim->arb_mutex);
	return (nearest);
}

int	heap_find(t_sim *sim, int coder_id)
{
	int	i;

	i = 0;
	while (i < sim->heap_size)
	{
		if (sim->heap[i].coder_id == coder_id)
			return (i);
		i++;
	}
	return (-1);
}

void	heap_remove(t_sim *sim, int idx)
{
	sim->heap_size--;
	if (idx < sim->heap_size)
	{
		sim->heap[idx] = sim->heap[sim->heap_size];
		heap_sift_up(sim->heap, idx);
		heap_sift_down(sim->heap, sim->heap_size, idx);
	}
}
