/* * Kostak - A little container engine
 * Copyright (C) 2020 Didiet Noor <dnoor@ykode.com>
 *
 * This file is part of Kostak.
 *
 * Kostak is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Kostak is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Kostak.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMPAT_H
#define COMPAT_H 1

#ifndef htonl
uint32_t htonl(uint32_t hostlong);
#endif

#ifndef htons
uint16_t htons(uint16_t hostshort);
#endif

#ifndef ntohl
uint32_t ntohl(uint32_t netlong);
#endif

#ifndef ntohs
uint16_t ntohs(uint16_t netshort);
#endif

#endif /* COMPAT_H */

