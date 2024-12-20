/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: copireyr <copireyr@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 12:46:57 by copireyr          #+#    #+#             */
/*   Updated: 2024/11/21 16:24:34 by copireyr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIGNALS_H
# define SIGNALS_H

# include <signal.h>

int		*exit_code(void);
int		set_signal_handler(void);
void	set_signal_handlers(void (*sigquit_fn)(int), void (*sigint_fn)(int));
void	redisplay_prompt(int sig);
void	handler_print_newline(int sig);

#endif /* SIGNALS_H */
