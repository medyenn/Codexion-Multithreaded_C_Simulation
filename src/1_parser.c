/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   1_parser.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mennih < mennih@student.1337.ma>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 14:26:04 by mennih            #+#    #+#             */
/*   Updated: 2026/09/06 20:07:43 by mennih           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

bool	is_valid_uint(const char *s)
{
	if (!s || *s == '\0')
		return (false);
	if (*s == '+')
		s++;
	if (*s == '\0')
		return (false);
	while (*s)
	{
		if (*s < '0' || *s > '9')
			return (false);
		s++;
	}
	return (true);
}

long long	safe_atoll(const char *s)
{
	long long	result;

	if (*s == '+')
		s++;
	result = 0;
	while (*s)
	{
		if (result > (long long)INT_MAX)
			return (-1);
		result = result * 10 + (*s - '0');
		s++;
	}
	return (result);
}

static int	parse_nums(int argc, char **argv, long long *vals)
{
	int	i;

	if (argc != 9)
	{
		fprintf(stderr, "Usage: %s n burnout compile debug"
			" refactor required cooldown scheduler\n", argv[0]);
		return (-1);
	}
	i = 0;
	while (i < 7)
	{
		if (!is_valid_uint(argv[i + 1]))
		{
			fprintf(stderr, "Error: arg %d must be >= 0.\n", i + 1);
			return (-1);
		}
		vals[i] = safe_atoll(argv[i + 1]);
		if (vals[i] < 0)
		{
			fprintf(stderr, "Error: arg %d overflows.\n", i + 1);
			return (-1);
		}
		i++;
	}
	return (0);
}

static int	parse_sched(char *s, t_sim *sim)
{
	if (strcmp(s, "fifo") == 0)
		sim->scheduler = POLICY_FIFO;
	else if (strcmp(s, "edf") == 0)
		sim->scheduler = POLICY_EDF;
	else
	{
		fprintf(stderr, "Error: scheduler must be 'fifo' or 'edf'.\n");
		return (-1);
	}
	return (0);
}

int	parse_args(int argc, char **argv, t_sim *sim)
{
	long long	vals[7];

	if (parse_nums(argc, argv, vals) != 0)
		return (-1);
	if (vals[0] < 1)
	{
		fprintf(stderr, "Error: number_of_coders must be >= 1.\n");
		return (-1);
	}
	if (vals[1] < 1)
	{
		fprintf(stderr, "Error: time_to_burnout must be >= 1 ms.\n");
		return (-1);
	}
	if (parse_sched(argv[8], sim) != 0)
		return (-1);
	sim->n = (int)vals[0];
	sim->time_to_burnout = vals[1];
	sim->time_to_compile = vals[2];
	sim->time_to_debug = vals[3];
	sim->time_to_refactor = vals[4];
	sim->compiles_required = (int)vals[5];
	sim->dongle_cooldown = vals[6];
	return (0);
}
