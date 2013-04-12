/*
 * Copyright(c) 1997-2001 Id Software, Inc.
 * Copyright(c) 2002 The Quakeforge Project.
 * Copyright(c) 2006 Quake2World.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "tests.h"
#include "mem.h"

/*
 * @brief Setup fixture.
 */
void setup(void) {
	Z_Init();
}

/*
 * @brief Teardown fixture.
 */
void teardown(void) {
	Z_Shutdown();
}

START_TEST(check_mem_LinkMalloc)
	{
		byte *parent = Z_Malloc(1);
		byte *child1 = Z_LinkMalloc(1, parent);
		byte *child2 = Z_LinkMalloc(1, parent);
		/*byte *grandchild1 =*/ Z_LinkMalloc(1, child1);

		ck_assert(Z_Size() == 4);

		Z_Free(child2);

		ck_assert(Z_Size() == 3);

		Z_Free(parent);

		ck_assert(Z_Size() == 0);

	}END_TEST

/*
 * @brief Test entry point.
 */
int main(int argc, char *argv[]) {

	Test_Init();

	TCase *tcase = tcase_create("check_mem");
	tcase_add_checked_fixture(tcase, setup, teardown);

	tcase_add_test(tcase, check_mem_LinkMalloc);

	Suite *suite = suite_create("check_mem");
	suite_add_tcase(suite, tcase);

	int failed = Test_Run(suite);

	Test_Shutdown();
	return failed != 0;
}
