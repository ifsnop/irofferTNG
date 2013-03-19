
struct db_row_t db_read(const char *query, ...);
int db_write(const char *query, ...);
void db_row_clean(struct db_row_t row);

