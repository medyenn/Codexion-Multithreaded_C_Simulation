/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils2.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mennih < mennih@student.1337.ma>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/07 00:00:00 by mennih            #+#    #+#             */
/*   Updated: 2026/09/07 00:00:00 by mennih           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	grant_request(t_coder *c)
{
	c->left->in_use = true;
	c->right->in_use = true;
	pthread_mutex_lock(&c->cond_mutex);
	c->granted = true;
	pthread_cond_signal(&c->cond);
	pthread_mutex_unlock(&c->cond_mutex);
}

/*
** Milliseconds left before coder i's burnout deadline, as of `now`.
** A coder that already reached its required compile count is no
** longer trying to compile at all, so it is reported as infinitely
** far from burning out instead of being checked for real - otherwise
** its frozen last_compile_start would eventually look overdue while
** it is simply done and waiting for the others.
*/
long long	coder_deadline_remain(t_sim *sim, int i, long long now)
{
	long long	start;
	long long	count;

	pthread_mutex_lock(&sim->coders[i].cond_mutex);
	start = sim->coders[i].last_compile_start;
	count = sim->coders[i].compile_count;
	pthread_mutex_unlock(&sim->coders[i].cond_mutex);
	if (count >= sim->compiles_required)
		return (1000000000LL);
	return (start + sim->time_to_burnout - now);
}

void	free_dongle_arrays(t_sim *sim)
{
	free(sim->dongles);
	sim->dongles = NULL;
	free(sim->heap);
	sim->heap = NULL;
	free(sim->pending);
	sim->pending = NULL;
}

void	handle_stop(t_sim *sim, int burned)
{
	sim_stop(sim);
	if (burned != 0)
		log_burnout(sim, burned);
	wake_all_coders(sim);
}

long long	monitor_tick(t_sim *sim)
{
	int			burned;
	long long	wake_us;
	long long	cooldown_us;

	if (sim->compiles_required > 0 && all_compiled(sim))
	{
		handle_stop(sim, 0);
		return (-1);
	}
	pthread_mutex_lock(&sim->arb_mutex);
	dispatch(sim);
	pthread_mutex_unlock(&sim->arb_mutex);
	burned = scan_coders(sim, &wake_us);
	if (burned != 0)
	{
		handle_stop(sim, burned);
		return (-1);
	}
	cooldown_us = next_cooldown_us(sim);
	if (cooldown_us < wake_us)
		wake_us = cooldown_us;
	return (wake_us);
}
