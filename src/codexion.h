/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mennih < mennih@student.1337.ma>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/06 22:10:37 by mennih            #+#    #+#             */
/*   Updated: 2026/09/07 22:59:52 by mennih           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <pthread.h>
# include <sys/time.h>
# include <stdbool.h>
# include <limits.h>

typedef enum e_scheduler
{
	POLICY_FIFO,
	POLICY_EDF
}	t_scheduler;

typedef struct s_sim		t_sim;
typedef struct s_dongle		t_dongle;
typedef struct s_coder		t_coder;
typedef struct s_request
{
	long long	priority;
	int			coder_id;
}	t_request;

struct s_dongle
{
	int			id;
	bool		in_use;
	long long	release_time;
};

struct s_coder
{
	int				id;
	t_dongle		*left;
	t_dongle		*right;
	long long		last_compile_start;
	int				compile_count;
	long long		ticket;
	bool			granted;
	bool			burned_out;
	t_sim			*sim;
	pthread_cond_t	cond;
	pthread_mutex_t	cond_mutex;
};

struct s_sim
{
	int				n;
	long long		time_to_burnout;
	long long		time_to_compile;
	long long		time_to_debug;
	long long		time_to_refactor;
	int				compiles_required;
	long long		dongle_cooldown;
	t_scheduler		scheduler;
	t_coder			*coders;
	t_dongle		*dongles;
	long long		start_time_ms;
	volatile bool	stopped;
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	stop_mutex;
	pthread_mutex_t	arb_mutex;
	t_request		*heap;
	int				heap_size;
	int				heap_cap;
	t_request		*pending;
	pthread_t		*threads;
	pthread_t		monitor_thread;
};

long long	get_time_ms(void);
long long	sim_elapsed(t_sim *sim);
void		log_event(t_sim *sim, int coder_id, const char *msg);
void		log_burnout(t_sim *sim, int coder_id);
int			msleep(long long ms);
bool		sim_is_stopped(t_sim *sim);
void		sim_stop(t_sim *sim);

bool		is_valid_uint(const char *s);
long long	safe_atoll(const char *s);

int			req_less(const t_request *a, const t_request *b);
void		heap_swap(t_request *heap, int i, int j);
void		heap_sift_up(t_request *heap, int idx);
void		heap_sift_down(t_request *heap, int size, int idx);
int			heap_push(t_sim *sim, t_request req);
void		heap_remove(t_sim *sim, int idx);
int			heap_find(t_sim *sim, int coder_id);

void		dongle_init(t_dongle *d, int id);
bool		dongle_ready(t_dongle *d, long long cooldown_ms);
void		dispatch(t_sim *sim);
void		dongle_request(t_sim *sim, t_coder *c);
void		dongle_release(t_sim *sim, t_coder *c);
bool		coder_wait_grant(t_sim *sim, t_coder *c);
long long	next_cooldown_us(t_sim *sim);

void		*coder_routine(void *arg);
void		put_dongles(t_coder *c);
void		*monitor_routine(void *arg);
void		wake_all_coders(t_sim *sim);
bool		all_compiled(t_sim *sim);
int			scan_coders(t_sim *sim, long long *wake_us);

void		grant_request(t_coder *c);
long long	coder_deadline_remain(t_sim *sim, int i, long long now);
void		free_dongle_arrays(t_sim *sim);
void		handle_stop(t_sim *sim, int burned);
long long	monitor_tick(t_sim *sim);

int			parse_args(int argc, char **argv, t_sim *sim);
int			sim_init(t_sim *sim);
void		sim_cleanup(t_sim *sim);
int			sim_run(t_sim *sim);

#endif
