/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mennih < mennih@student.1337.ma>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 14:26:25 by mennih            #+#    #+#             */
/*   Updated: 2026/09/02 14:26:26 by mennih           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long long)tv.tv_sec * 1000LL + (long long)tv.tv_usec / 1000LL);
}

long long	sim_elapsed(t_sim *sim)
{
	return (get_time_ms() - sim->start_time_ms);
}

void	log_event(t_sim *sim, int coder_id, const char *msg)
{
	long long	ts;

	pthread_mutex_lock(&sim->log_mutex);
	ts = sim_elapsed(sim);
	printf("%lld %d %s\n", ts, coder_id, msg);
	pthread_mutex_unlock(&sim->log_mutex);
}

void	log_burnout(t_sim *sim, int coder_id)
{
	long long	ts;

	ts = sim_elapsed(sim);
	pthread_mutex_lock(&sim->log_mutex);
	printf("%lld %d burned out\n", ts, coder_id);
	pthread_mutex_unlock(&sim->log_mutex);
}

int	msleep(long long ms)
{
	long long	deadline;
	long long	now;
	long long	remaining;

	deadline = get_time_ms() + ms;
	while (1)
	{
		now = get_time_ms();
		if (now >= deadline)
			return (0);
		remaining = deadline - now;
		if (remaining > 1000)
			remaining = 1000;
		usleep((useconds_t)(remaining * 1000LL));
	}
}
