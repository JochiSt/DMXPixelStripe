/*-----------------------------------------------------------------------------
 Dieses Programm ist freie Software. Sie können es unter den Bedingungen der
 GNU General Public License, wie von der Free Software Foundation veröffentlicht,
 weitergeben und/oder modifizieren, entweder gemäß Version 2 der Lizenz oder
 (nach Ihrer Option) jeder späteren Version.

 Die Veröffentlichung dieses Programms erfolgt in der Hoffnung,
 daß es Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE,
 sogar ohne die implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT
 FÜR EINEN BESTIMMTEN ZWECK. Details finden Sie in der GNU General Public License.

 Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 Programm erhalten haben.
 Falls nicht, schreiben Sie an die Free Software Foundation,
 Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
------------------------------------------------------------------------------*/

#ifndef _CONFIG_H_
	#define _CONFIG_H_
	#define F_CPU 			16000000

	#define DMX_LOST_TIMEOUT 	500

	#define M0	B,0
	#define M1	B,1
	#define M2	B,2

	#define A8	D,7
	#define A7	D,6
	#define A6	D,5
	#define A5	C,0
	#define A4	C,1
	#define A3	C,2
	#define A2	C,3
	#define A1	C,4
	#define A0	C,5

#endif //_CONFIG_H

