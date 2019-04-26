#ifndef LEXER_H
#define LEXER_H

char	*read_text_from(char *name); //Don't check '\n' at EOF
void		read_table(char *name, t_fsm *fsm);

void	add_to_op_list(t_op_list **op_list, t_token_list *token_list);
void	add_to_token_list(t_token_list **token_list, t_identifier ident,
						  void *data, t_asm *a);
void	add_to_label_list(t_label_list **label_list, char *name, t_asm *a);
void	reverse_op_list(t_op_list **op_list);
void	reverse_token_list(t_token_list **token_list);

#endif