import { AppSidebarTrigger } from '@/components/layout/sidebar';
import { Separator } from '@/components/ui/separator';
import { SquareTerminal, LogOut } from 'lucide-react';
import { useMatches } from '@tanstack/react-router';
import { menuItems } from '../sidebar/app-sidebar';

export function AppHeader() {
  const matches = useMatches();

  // Get the current deepest match that corresponds to one of our menu items
  const found = [...matches]
    .reverse()
    .find((match) => menuItems.some((item) => item.url === match.pathname));

  const title = found
    ? menuItems.find((item) => item.url === found.pathname)?.title
    : 'ESP Dragon Fruit';

  return (
    <header className='bg-sidebar sticky top-0 h-14 border-b px-4 flex items-center'>
      <div className='flex-1 flex justify-between'>
        <div className='flex items-center'>
          <AppSidebarTrigger />
          <h1 className='ml-3 text-lg'>{title}</h1>
        </div>
        <div className='flex items-center gap-3'>
          <SquareTerminal />
          <Separator orientation='vertical' />
          <LogOut />
        </div>
      </div>
    </header>
  );
}
