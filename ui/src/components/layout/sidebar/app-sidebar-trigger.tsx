import { useSidebar } from '@/components/ui/sidebar';
import { Button } from '@/components/ui/button';
import { Menu, X } from 'lucide-react';

interface AppSidebarTriggerProps {
  isMobileSidebar?: boolean;
}

export function AppSidebarTrigger({
  isMobileSidebar = false
}: AppSidebarTriggerProps) {
  const { toggleSidebar } = useSidebar();

  return (
    <Button
      data-sidebar='trigger'
      data-slot='sidebar-trigger'
      variant='ghost'
      size='icon'
      className='size-7'
      onClick={toggleSidebar}
    >
      {isMobileSidebar ? <X /> : <Menu />}
      <span className='sr-only'>Toggle Sidebar</span>
    </Button>
  );
}
